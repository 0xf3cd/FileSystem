#ifndef INODEMANAGER
#define INODEMANAGER

#include "INode.h"
#include "BufferManager.h"
#include "DeviceDriver.h"
#include "SuperBlockManager.h"
#include <set>
using namespace std;

/**
 * 这个类的作用主要是用于管理内存 INode 节点
 * 需要创建、打开、更改、删除文件时，需要通过这个类获取节点
 * 这个类记录 INode 共享情况，即可能有多个文件共享一个 INode
 * 这个类提供 打开、创建、销毁 INode 的接口，也提供将 INode 写回外存的接口、从外存读取 INode 的接口
 */
class INodeManager {
private:
    set<MemINode*> Iset;
    BufferManager* BM;
    DeviceDriver* DD;
    SuperBlockManager* SBM;

public:
    INodeManager();
    ~INodeManager();

    /**
     * 根据外存的 inode 编号，找到并读取到内存中
     */
    MemINode* readDINode(int ino);

    /**
     * 分配一个新的内存 inode
     */
    MemINode* getNewMINode();

    /**
     * 释放一个内存 inode
     * 如果 inode 只被一个 File 结构使用，则从 iset 中清除
     * 如果有 inode 共享的情况，则不释放
     */
    void freeMINode(MemINode* pinode);

    /**
     * 写回一个内存 inode 到外存上
     */
    void writeBackMINode(MemINode* pinode);

    /**
     * 检查一个外存 inode 是否已经读入内存
     * 传入外存 inode 编号
     */
    bool hasLoadedDINode(int ino);

    /**
     * 在 Iset 中查找已经读入内存的外存 inode
     * 传入外存 inode 编号
     * 返回找到的 inode 指针
     */
    MemINode* getLoadedDINode(int ino);
};

#endif