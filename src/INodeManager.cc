#include "INodeManager.h"
#include "BufferManager.h"
#include "DeviceDriver.h"
#include "FileSystem.h"
#include <iostream>

extern BufferManager g_BufferManager;
extern DeviceDriver g_DeviceDriver;
extern FileSystem g_FileSystem;

INodeManager::INodeManager() {
    BM = &g_BufferManager;
    DD = &g_DeviceDriver;
    FS = &g_FileSystem;
}

INodeManager::~INodeManager() {
    set<MemINode*>::iterator it;
    for(it = Iset.begin(); it != Iset.end(); it++) {
        if((*it) -> m_mode & MemINode::IUPD) {
            writeBackMINode(*it);
        }
        delete *it;
    }
}

/**
 * 根据外存的 inode 编号，找到并读取到内存中
 */
MemINode* INodeManager::readDINode(int ino) {
    MemINode* minode;

    // cout << Iset.size() << endl;
    if(!FS -> hasAllocedDINode(ino)) {
        cout << ino << " 外存 inode 未被使用！" << endl;
        return nullptr; // 如果外存 inode 未被分配，则返回空指针
    }

    if(hasLoadedDINode(ino)) {
        // 如果已经读取过这个 inode，且 Iset 中存在对应内存 inode
        minode = getLoadedDINode(ino);
        minode -> m_count += 1;
        return minode;
    }

    DiskINode dinode;
    minode = new MemINode;
    DD -> read((char*)&dinode, sizeof(DiskINode), sizeof(SuperBlock) + ino * sizeof(DiskINode));
    minode -> readDiskINode(dinode);
    minode -> m_number = ino;
    Iset.insert(minode);

    return minode;
}

/**
 * 分配一个新的内存 inode
 */
MemINode* INodeManager::getNewMINode() {
    const int ino = FS -> allocDiskINode();
    DiskINode dinode;
    dinode.d_mode |= DiskINode::IALLOC;

    MemINode* minode = new MemINode;
    minode -> readDiskINode(dinode);
    minode -> m_mode |= MemINode::IUPD; // 设定标志位，指示这个 inode 需要被写回外存
    minode -> m_count = 1;
    minode -> m_number = ino;

    Iset.insert(minode);

    // cout << Iset.size() << endl;
    return minode;
}

/**
 * 释放一个内存 inode
 * 如果 inode 只被一个 File 结构使用，则从 Iset 中清除
 * 如果有 inode 共享的情况，则不释放
 */
void INodeManager::freeMINode(MemINode* pinode) {
    if(pinode -> m_count > 1) {
        // 说明还有其他 File 结构在使用这个 inode
        pinode -> m_count -= 1;
    } else {
        // 说明没有其他 File 结构在使用这个 inode，可以释放掉
        if(pinode -> m_mode & MemINode::IUPD) {
            writeBackMINode(pinode);
        }
        Iset.erase(pinode);
        delete pinode;
    }
}

/**
 * 写回一个内存 inode 到外存上
 */
void INodeManager::writeBackMINode(MemINode* pinode) {
    pinode -> m_mode &= ~MemINode::IUPD;
    const int ino = pinode -> m_number;
    const DiskINode dinode = pinode -> getDiskINode();
    DD -> write((char*)&dinode, sizeof(DiskINode), sizeof(SuperBlock) + ino * sizeof(DiskINode));
}

/**
 * 检查一个外存 inode 是否已经读入内存
 * 传入外存 inode 编号
 */
bool INodeManager::hasLoadedDINode(int ino) {
    set<MemINode*>::iterator it;
    for(it = Iset.begin(); it != Iset.end(); it++) {
        if((*it) -> m_number == ino) {
            return true;
        }
    }
    return false;
}

/**
 * 在 Iset 中查找已经读入内存的外存 inode
 * 传入外存 inode 编号
 * 返回找到的 inode 指针
 */
MemINode* INodeManager::getLoadedDINode(int ino) {
    set<MemINode*>::iterator it;
    for(it = Iset.begin(); it != Iset.end(); it++) {
        if((*it) -> m_number == ino) {
            return *it;
        }
    }
    return nullptr;
}