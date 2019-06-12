#include "INode.h"
#include "BufferManager.h"
#include "FileSystem.h"
#include "File.h"
#include <iostream>

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;

File::File(MemINode* minode) {
    BM = &g_BufferManager;
    FS = &g_FileSystem;

    f_type = 0;
    if(minode -> m_mode & MemINode::IFDIR) {
        f_type |= File::FDIR;
    }
    f_minode = minode;
    f_offset = 0;
}

File::~File() {
    // inode 不在此释放
    // 由使用这个 File 对象的使用者释放
}

/**
 * 获取文件大小
 */
int File::getFileSize() {
    return f_minode -> m_size;
}

/**
 * 获取文件使用的盘块数
 */
int File::getBlkNum() {
    const int fsize = f_minode -> m_size;
    int blk_num = fsize / 512;
    if(fsize % 512 != 0) {
        blk_num++;
    }
    return blk_num;
}

/**
 * 得到当前文件使用的最后一块盘块剩余容量
 */
int File::getLastBlkRest() {
    const int fsize = f_minode -> m_size;
    return fsize % 512;
}

/**
 * 为文件新申请一个盘块
 * 返回相应缓存块
 */
Buffer* File::applyNewBLlk() {
    const int pre_blk_num = getBlkNum();
    if(pre_blk_num >= HUGE_FILE_MOST_BLK) {
        cout << "文件过大！" << endl;
        return nullptr;
    }

    Buffer* buf = FS -> allocBlock();
    const int blk_no = buf -> b_blk_no;
    int* addr = f_minode -> m_addr;

    // pre_blk_num 为申请新块之前的块数
    if(pre_blk_num < SMALL_FILE_MOST_BLK) { // 0 - 5 个盘块，不需要索引
        addr[pre_blk_num] = blk_no;
    } else if(pre_blk_num < LARGE_FILE_MOST_BLK) { // （6 - 6 + 128 * 2） 个盘块，一次间接索引
        int indexed_blk_num = pre_blk_num + 1 - 6; // 除去前六个盘块，需要进行索引的盘块数
        if(indexed_blk_num % 128 == 1) {
            // 如果在某个索引块未被创建

        }

    } else if(pre_blk_num < HUGE_FILE_MOST_BLK){ // 需要二次间接索引
        int indexed_blk_num = pre_blk_num + 1 - 6 - 128 * 2; // 除去前面不需要索引及一级索引的盘块，需要进行索引的盘块数
        if(indexed_blk_num % (128 * 128) == 1) {
            // 如果一级索引块未被创建
            
        }

        if(indexed_blk_num % 128 == 1) {
            // 如果二级索引块未被创建

        }
    }

    return buf;
}

/**
 * 进行逻辑盘块号到物理盘块号的映射
 */
int File::mapBlk(int lbn) {
    int bn;
    const int* addr = f_minode -> m_addr;
    const int fsize = f_minode -> m_size;
    const int blk_num = getBlkNum();

    if(lbn > blk_num || lbn < 0) {
        cout << "文件没有第 " << lbn << " 个逻辑块" << endl;
        return -1;
    }

    if(lbn < SMALL_FILE_MOST_BLK) { // 0 - 5 个盘块，不需要索引
        return addr[lbn];
    } else if(lbn < LARGE_FILE_MOST_BLK) { // （6 - 6 + 128 * 2） 个盘块，一次间接索引

    } else if(lbn < HUGE_FILE_MOST_BLK){ // 需要二次间接索引

    } else {
        cout << "文件过大，" << "检查 File.cc 中的 mapBlk 函数" << endl;
        return -1;
    }

    return bn;
}

/**
 * 删除文件的所有内容
 */
void File::trunc() {
    f_minode -> m_mode |= MemINode::IUPD;
}

/**
 * 读取文件内容
 * 以当前 f_offset 为起始字节开始读取
 * 返回读取的字节数
 */
int File::read(char* content, int length) {

}

/**
 * 将内容写入文件
 * 以当前 f_offset 为起始字节开始写入
 * 返回写入的字节数
 */
int File::write(char* content, int length) {
    f_minode -> m_mode |= MemINode::IUPD;
}

/**
 * 关闭文件
 */
void File::close() {

}