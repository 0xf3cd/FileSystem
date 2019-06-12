#include "Buffer.h"
#include "BufferManager.h"
#include "DeviceDriver.h"
#include <iostream>

extern DeviceDriver g_DeviceDriver;

/**
 * 初始化所有缓存块
 */
void BufferManager::initialize() {
    int i;
    for(i = 0; i < BUF_NUM; i++) {
        if(i == 0) { // 第一个块
            buf[i].b_forw = &free_list;
            free_list.b_back = &buf[i];
        } else {
            buf[i].b_forw = &buf[i-1];
        }

        if(i == BUF_NUM - 1) { // 最后一个块
            buf[i].b_back = &free_list;
            free_list.b_forw = &buf[i];
        } else {
            buf[i].b_back = &buf[i+1];
        }

        buf[i].b_addr = buffer[i];
    }
}

BufferManager::BufferManager() {
    DD = &g_DeviceDriver;
    initialize();
}

BufferManager::~BufferManager() {
    flushAllBuf();
}

/**
 * 从队列中摘取一个缓存块
 */
void BufferManager::detachBuf(Buffer* buf) {
    if(buf -> b_back == nullptr) {
        return;
    }

    buf -> b_forw -> b_back = buf -> b_back;
    buf -> b_back -> b_forw = buf -> b_forw;

    buf -> b_back = nullptr;
    buf -> b_forw = nullptr;
}

/**
 * 将一个缓存块加入队尾
 */
void BufferManager::insertBuf(Buffer* buf) {
    if(buf -> b_back != nullptr) {
        return;
    }

    buf -> b_forw = free_list.b_forw;
    buf -> b_back = &free_list;

    free_list.b_forw -> b_back = buf;
    free_list.b_forw  = buf;

    // cout << (free_list.b_back == &free_list) << endl;
}

/**
 * 申请一块缓存，用于读写磁盘上的块
 */
Buffer* BufferManager::getBuf(int blk_no) {
    Buffer* buf;

    if(map.find(blk_no) != map.end()) { // 如果之前有某 buf 读取过这个盘块
        buf = map[blk_no];
        detachBuf(buf);
        return buf;
    }

    if(free_list.b_back == &free_list) { // 如果队列空了
        cout << "Error of buf" << endl;
        return nullptr;
    }

    buf = free_list.b_back;
    detachBuf(buf);

    if(buf -> b_flags & Buffer::B_DELWRI) { // 如果设置了延迟写
        DD -> write(buf -> b_addr, BUF_SIZE, buf -> b_blk_no * 512);
    }

    map.erase(buf -> b_blk_no); // 删除这块缓存之前读的盘块的记录
    map[blk_no] = buf; // 在记录中加上新的读取记录
    buf -> b_blk_no = blk_no;
    buf -> b_flags &= ~(Buffer::B_DELWRI | Buffer::B_DONE); // 消除之前的标志位

    return buf;
}

/**
 * 释放缓存控制块 buf
 */
void BufferManager::freeBuf(Buffer* buf) {
    insertBuf(buf);
}

/**
 * 读磁盘的 blk_no 盘块
 */
Buffer* BufferManager::readBuf(int blk_no) {
    Buffer* buf = getBuf(blk_no);
    if(buf -> b_flags & (Buffer::B_DELWRI | Buffer::B_DONE)) {
        // 如果某个块是之前读取过相关盘块，或是设置了延迟写，则直接返回
        return buf;
    }

    DD -> read(buf -> b_addr, BUF_SIZE, buf -> b_blk_no * 512);
    return buf;
}

/**
 * 写一个盘块
 */
void BufferManager::writeBuf(Buffer* buf) {
    buf -> b_flags &= ~Buffer::B_DELWRI; // 清除延迟写标志位
	DD -> write(buf -> b_addr, BUF_SIZE, buf -> b_blk_no * 512);
	buf -> b_flags |= Buffer::B_DONE; 

	freeBuf(buf);
}

/**
 * 延迟写一个盘块
 */
void BufferManager::dwriteBuf(Buffer* buf) {
    buf -> b_flags |= Buffer::B_DELWRI;
	buf -> b_flags |= Buffer::B_DONE; 

	freeBuf(buf);
}

/**
 * 清空这个 buf 中的内容
 */
void BufferManager::clearBuf(Buffer* buf) {
    int i;
    for(i = 0; i < BUF_SIZE; i++) {
        (buf -> b_addr)[i] = 0;
    }
}

/**
 * 将延迟写的缓存块中的内容全部写入磁盘
 */
void BufferManager::flushAllBuf() {
    int i;
    for(i = 0; i < BUF_NUM; i++) {
        if(buf[i].b_flags & Buffer::B_DELWRI) { // 如果设置了延迟写
            buf[i].b_flags &= ~Buffer::B_DELWRI;
            DD -> write(buf[i].b_addr, BUF_SIZE, buf[i].b_blk_no * 512);
            buf[i].b_flags |= Buffer::B_DONE;
        }
    }
}