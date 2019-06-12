#include "SuperBlock.h"

SuperBlock::SuperBlock() {
    int i;

    s_isize = INODE_NUM; // inode 总个数    
    // char s_ibitmap[128]; // 记录 inode 分配情况的位图
    for(i = 0; i < 128; i++) {
        s_ibitmap[i] = 0;
    }

    s_ilock = 0; // 临界区锁

    s_fsize = DISK_BLOCK_NUM; // 盘块总数
    s_nfree = 100; // 直接管理的空闲盘块数

    // int s_free[100]; // 直接管理的空闲盘块的索引表
    for(i = 0; i < 100; i++) {
        s_free[i] = 0;
    }

    s_flock = 0; // 临界区锁
    
    s_dirty = 0; // 指示内存中的超级块是否被修改，是否需要写回外存
}

/**
 * 格式化超级块
 */
// void SuperBlock::format() {

// }