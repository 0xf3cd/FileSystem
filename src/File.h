#ifndef FILE
#define FILE

#include "INode.h"
#include "BufferManager.h"
#include "SuperBlockManager.h"

#define SMALL_FILE_MOST_BLK 6
#define LARGE_FILE_MOST_BLK 6 + 128 * 2
#define HUGE_FILE_MOST_BLK 6 + 128 * 2 + 128 * 128 * 2

/**
 * 这个类主要根据传入的 MemINode 节点打开一个文件
 * 通过调用这个类提供的方法，可以读写或删除文件内容
 */
class File {
public:
    enum FileType { 
        FDIR = 0x1 // 是否是文件夹
    };

    SuperBlockManager* SBM;
    BufferManager* BM;

    int f_type;
    MemINode* f_minode;
    int f_offset;

private:
public:
    /**
     * 为文件新申请一个盘块
     * 返回相应缓存块
     */
    Buffer* applyNewBlk();

    /**
     * 将文件截断
     * 根据输入，保留文件前面若干字节
     */
    void trunc(const int size);

public:
    File(MemINode* minode);
    ~File();

    /**
     * 获取文件大小
     */
    int getFileSize();

    /**
     * 获取文件使用的盘块数
     */
    int getBlkNum();

    /**
     * 得到当前文件使用的最后一块盘块剩余容量
     */
    int getLastBlkRest();

    /**
     * 进行逻辑盘块号到物理盘块号的映射
     */
    int mapBlk(int lbn);

    /**
     * 删除文件的所有内容
     */
    void deleteAll();

    /**
     * 读取文件内容
     * 以当前 f_offset 为起始字节开始读取
     * 返回读取的字节数
     */
    int read(char* content, int length);

    /**
     * 将内容写入文件
     * 以当前 f_offset 为起始字节开始写入
     * 返回写入的字节数
     */
    int write(char* content, int length);

    /**
     * 删除文件中一部分内容
     * 以当前 f_offset 为起始开始删除
     * 删除的内容保存在 content 中（如果 content 不为 nullptr）
     * content 为 nullptr 则不保存删除的内容
     * 返回删除的字节数
     */
    int remove(char* content, int length);
};

#endif