#include "SuperBlockManager.h"
#include "DeviceDriver.h"
#include "SuperBlock.h"
#include "INode.h"
#include "BufferManager.h"
#include "DirectoryEntry.h"

#include <iostream>
#include <string.h>

extern DeviceDriver g_DeviceDriver;
extern SuperBlock g_SuperBlock;
extern BufferManager g_BufferManager;

SuperBlockManager::SuperBlockManager() {
    DD = &g_DeviceDriver;
    SB = &g_SuperBlock;
    BM = &g_BufferManager;
    loadSuperBlock();
}

SuperBlockManager::~SuperBlockManager() {
    saveSuperBlock();
}

/**
 * 读取磁盘中的 SuperBlock 
 */
void SuperBlockManager::loadSuperBlock() {
    SuperBlock new_SB;
    DD -> read((char*)&new_SB, 1024, 0);
    *SB = new_SB;
}

/**
 * 保存 SuperBlock 到磁盘
 */
void SuperBlockManager::saveSuperBlock() {
    if(SB -> s_dirty) {
        SB -> s_dirty = 0;
        DD -> write((char*)SB, 1024, 0);
    }
}

/**
 * 重置 SuperBlock 中空闲盘块的相关记录信息
 * 使用成组链接法
 */
void SuperBlockManager::resetFreeBlockInfo() {
    int disk_num = DISK_BLOCK_NUM - (2 + INODE_NUM / 8);
    int group_num = disk_num / 100; // 分成的组数，每一组 100 个空闲盘块
    int rest_num = disk_num % 100;
    const int start_blk_no = 2 + INODE_NUM / 8; // 2 for SuperBlock
    int blk_count = start_blk_no;
    int next_group_head_no;
    int i, j;

    if(rest_num != 0) {
        group_num++; // 如果剩下少于 100 个盘块，则需要另外分一组
    }

    // 没有考虑盘块小于 100 的情况！
    SB -> s_nfree = 100; 
    next_group_head_no = blk_count;
    for(i = 0; i < 100; i++) {
        SB -> s_free[i] = blk_count;
        blk_count++;
    }

    // 生成各个组块的信息
    for(i = 1; i < group_num-1; i++) {
        int content[101];
        content[0] = 100;

        for(j = 1; j <= 100; j++) {
            content[j] = blk_count;
            blk_count++;
        }

        // for(int k = 0; k < 101; k++) {
        //     cout << content[k] << endl;
        // }
        // cout << endl;

        DD -> write((char*)content, sizeof(content), next_group_head_no*512);
        next_group_head_no = content[1]; // 记录下一次信息应该保存在哪块盘块上
    }

    int content[101] = {0};
    content[0] = rest_num;
    content[1] = 0;

    for(i = 2; i <= rest_num+1; i++) {
        content[i] = blk_count;
        blk_count++;
    }
    
    DD -> write((char*)content, sizeof(content), next_group_head_no*512);
}

/**
 * 为清空后的磁盘进行根目录创建工作
 * 创建外存 inode，并创建好相关 DirectoryEntry 项（如 . ..）
 */
void SuperBlockManager::createRootDir() {
    const int ino = allocDiskINode();
    Buffer* blk = allocBlock();
    int bno = blk -> b_blk_no;
    char* buf = blk -> b_addr;

    DiskINode dinode;
    dinode.d_addr[0] = bno;
    dinode.d_size = 2 * sizeof(DirectoryEntry);
    dinode.d_mode |= DiskINode::IALLOC;
    dinode.d_mode |= DiskINode::IFDIR;
    DD -> write((char*)&dinode, sizeof(DiskINode), sizeof(SuperBlock) + ino * sizeof(DiskINode)); // 将根目录的 inode 节点写入外存

    DirectoryEntry dir[2];
    memcpy(dir[0].fname, ".", 2);
    memcpy(dir[1].fname, "..", 3);
    dir[0].ino = ino;
    dir[1].ino = ino;

    // const char* t = (char*)(dir);
    // for(int i = 0; i < sizeof(dir); i++) {
    //     cout << t[i];
    // }

    memcpy(buf, (char*)dir, sizeof(dir));
    BM -> writeBuf(blk);
}

/**
 * 检查一个外存 inode 是否已经分配
 * 传入 inode 节点标号
 */
bool SuperBlockManager::hasAllocedDINode(int ino) {
    const unsigned char one = 1;
    int row, col;

    row = ino / 8;
    col = ino % 8;
    unsigned char tmp = (SB -> s_ibitmap)[row] & (one << col); // 为 0 说明没有分配，否则说明分配

    return tmp != 0; 
}

/**
 * 分配一个 inode 节点
 * 返回外存 inode 编号
 */
int SuperBlockManager::allocDiskINode() {
    const int bitmap_size = IBITMAP_SIZE;
    int i, j;

    bool find = false;
    int ino = -1;
    const unsigned char one = 1;
    char line;
    for(i = 0; i < bitmap_size && !find; i++) {
        line = (SB -> s_ibitmap)[i];
        for(j = 0; j < 8; j++) {
            if(!((one << j) & line)) { // 如果某一位为 0
                find = true;
                ino = 8 * i + j;

                SB -> s_dirty = 1;
                (SB -> s_ibitmap)[i] |= (one << j);
                break;
            }
        }
    }

    if(ino == -1) {
        cout << "外存 inode 节点用尽" << endl;
    }
    // cout << ino << endl;
    // cout << "分配" << ino << endl;
    return ino;
}

/**
 * 释放一个 inode 节点
 * 传入 inode 节点编号
 */
void SuperBlockManager::freeDiskINode(int no) {
    const unsigned char one = 1;
    int row, col;

    row = no / 8;
    col = no % 8;
    SB -> s_dirty = 1;
    (SB -> s_ibitmap)[row] &= ~(one << col);
}

/**
 * 分配一个 block 盘块
 * 返回一个缓存块的指针
 */
Buffer* SuperBlockManager::allocBlock() {
    Buffer* buf;
    int i;

    SB -> s_dirty = 1;
    int blk_no = (SB -> s_free)[SB -> s_nfree - 1];
    SB -> s_nfree -= 1;

    if(SB -> s_nfree == 0) {
        int content[101];
        buf = BM -> readBuf(blk_no);

        // DD -> read((char*)content, sizeof(content), 512*blk_no);
        for(i = 0; i < 101; i++) {
            content[i] = ((int*)buf -> b_addr)[i];
        }

        SB -> s_nfree = content[0];
        for(int i = 0; i < 100; i++) {
            (SB -> s_free)[i] = content[i+1];
        }

        BM -> freeBuf(buf);
    }

    // cout << blk_no << endl;
    buf = BM -> getBuf(blk_no);
    return buf;
}

/**
 * 释放一个盘块
 * 传入盘块编号
 */
void SuperBlockManager::freeBlock(int no) {
    Buffer* buf;
    SB -> s_dirty = 1;

    if(SB -> s_nfree == 100) {
        buf = BM -> getBuf(no);

        int content[101];
        content[0] = SB -> s_nfree;
        for(int i = 0; i < 100; i++) {
            content[i+1] = (SB -> s_free)[i];
        }
        // DD -> write((char*)content, sizeof(content), 512*no);
        BM -> writeBuf(buf);

        SB -> s_nfree = 0;
    }

    (SB -> s_free)[SB -> s_nfree] = no;
    SB -> s_nfree += 1;
}

/**
 * 格式化整个磁盘
 */
void SuperBlockManager::formatDisk() {
    char temp[512] = {0};
    int i;

    // 用 0 覆盖写，将磁盘清空
    for(i = 0; i < DISK_BLOCK_NUM; i++) {
        DD -> write(temp, 512, i*512);
    }

    DiskINode new_DI;
    for(i = 0; i < INODE_NUM; i++) {
        DD -> write((char*)&new_DI, 64, 1024+64*i);
    }

    SuperBlock new_SB;
    *SB = new_SB;

    resetFreeBlockInfo();
    createRootDir();
    saveSuperBlock();
}