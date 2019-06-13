#include "./DeviceDriver.h"
#include "./SuperBlock.h"
#include "./INode.h"
#include "./BufferManager.h"
#include "./DirectoryEntry.h"

#ifndef FILESYSTEM
#define FILESYSTEM

/**
 * 这个类提供了对 SuperBlock 管理的各方法
 * 提供格式化磁盘、分配外存 inode 节点、分配一个盘块等方法
 */
class FileSystem {
private:
    DeviceDriver* DD;
    SuperBlock* SB;
    BufferManager* BM;

private:
    /**
     * 读取磁盘中的 SuperBlock 
     */
    void loadSuperBlock();

    /**
     * 保存 SuperBlock 到磁盘
     */
    void saveSuperBlock();

    /**
     * 重置 SuperBlock 中空闲盘块的相关记录信息
     * 使用成组链接法
     */
    void resetFreeBlockInfo();

    /**
     * 为清空后的磁盘进行根目录创建工作
     * 创建外存 inode，并创建好相关 DirectoryEntry 项（如 . ..）
     */
    void createRootDir();

public: 
    FileSystem();
    ~FileSystem();

    /**
     * 格式化整个磁盘
     */
    void formatDisk();

    /**
     * 检查一个外存 inode 是否已经分配
     * 传入 inode 节点标号
     */
    bool hasAllocedDINode(int ino);

    /**
     * 分配一个 disk inode 节点
     * 返回外存 inode 编号
     */
    int allocDiskINode(); 

    /**
     * 释放一个 disk inode 节点
     * 传入 inode 节点编号
     */
    void freeDiskINode(int no);

    /**
     * 分配一个 block 盘块
     * 返回一个缓存块的指针
     */
    Buffer* allocBlock();

    /**
     * 释放一个盘块
     * 传入盘块编号
     */
    void freeBlock(int no);
};

#endif