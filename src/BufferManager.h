#ifndef BUFFERMAMAGER
#define BUFFERMAMAGER

#include "Buffer.h"
#include "DeviceDriver.h"
#include <map>
using namespace std;

#define BUF_NUM 100 // 一共 100 个缓冲块
#define BUF_SIZE 512  // 缓冲区大小 512 字节

/**
 * 这个类是对缓存块进行管理和调度的类
 * 提供了方法，能够利用高速缓存读写盘块
 */
class BufferManager {
private: 
    DeviceDriver* DD;

    Buffer free_list;	// 自由队列
    Buffer buf[BUF_NUM]; // 缓存控制块
    char buffer[BUF_NUM][BUF_SIZE]; // 缓冲区
    map<int, Buffer*> map; // 记录每个 buf 与所读取的盘块号之间的对应关系

private:
    /**
     * 初始化所有缓存块
     */
    void initialize();

    /**
     * 从队列中摘取某个缓存块
     */
    void detachBuf(Buffer* buf);

    /**
     * 将一个缓存块加入队尾
     */
    void insertBuf(Buffer* buf);

public: 
    BufferManager();
    ~BufferManager();

    /**
     * 申请一块缓存，用于读写磁盘上的块
     */
    Buffer* getBuf(int blk_no);
    
    /**
     * 释放缓存控制块 buf
     */
	void freeBuf(Buffer* buf);

    /**
     * 读磁盘的 blk_no 盘块
     */
    Buffer* readBuf(int blk_no);

    /**
     * 写一个盘块
     */
    void writeBuf(Buffer* buf);
    
    /**
     * 延迟写一个盘块
     */
	void dwriteBuf(Buffer* buf);

    /**
     * 清空这个 buf 中的内容
     */
    void clearBuf(Buffer* buf);
    
    /**
     * 将延迟写的缓存块中的内容全部写入磁盘
     */
	void flushAllBuf();

    /**
     * 
     */
};

#endif