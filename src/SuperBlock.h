#ifndef SUPERBLOCK
#define SUPERBLOCK

#define INODE_NUM 1024
#define IBITMAP_SIZE 128
#define DISK_BLOCK_NUM 64 * 1024

class SuperBlock {
public:
    int s_isize; // inode 总个数
    char s_ibitmap[IBITMAP_SIZE]; // 记录 inode 分配情况的位图
    // int s_ninode; // 直接管理的空闲的 inode 数量
    // int s_inode[100]; // 直接管理的空闲的 inode 的索引表
    int s_ilock; // 临界区锁

    int s_fsize; // 盘块总数
    int s_nfree; // 直接管理的空闲盘块数
    int s_free[100]; // 直接管理的空闲盘块的索引表
    int s_flock; // 临界区锁

    int s_dirty; // 指示是否需要写回磁盘

    char padding[472]; // 填充至 1024 字节

public:
    SuperBlock();

    /**
     * 格式化超级块
     */
    // void format();
};

#endif