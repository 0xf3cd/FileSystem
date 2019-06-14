#ifndef DISKINODE
#define DISKINODE

class DiskINode {
public:
    enum INodeFlags {
        IUPD = 0x2,		    /* 内存 inode 被修改过，需要更新外存 inode */
        IACC = 0x4,		    /* 内存 inode 被访问过，需要修改最近一次访问时间 */
        IALLOC = 0x8000,	/* 文件被使用 */
        IFDIR = 0x4000,		/* 文件类型：目录文件 */
        ILARG = 0x1000		/* 文件长度类型：大型或巨型文件 */
    };

    unsigned int d_mode; // 状态的标志位
    int d_nlink; // 文件在目录中不同路径的数量
    short d_uid; // 文件所有者的用户标识数
    short d_gid;  // 文件所有者的组标识数
    int d_size; // 文件大小，字节为单位
    int d_addr[10]; // 文件的盘块的混合索引表
    int d_atime; // 最后访问时间
    int d_mtime; // 最后修改时间

public:
    DiskINode();
};

#endif

#ifndef MEMINODE
#define MEMINODE

class MemINode {
public:
    enum INodeFlags {
        IUPD = 0x2,		    /* 内存 inode 被修改过，需要更新外存 inode */
        IACC = 0x4,		    /* 内存 inode 被访问过，需要修改最近一次访问时间 */
        IALLOC = 0x8000,	/* 文件被使用 */
        IFDIR = 0x4000,		/* 文件类型：目录文件 */
        ILARG = 0x1000		/* 文件长度类型：大型或巨型文件 */
    };

    unsigned int m_mode; // 状态的标志位
    int m_nlink; // 文件在目录中不同路径的数量
    short m_uid; // 文件所有者的用户标识数
    short m_gid;  // 文件所有者的组标识数
    int m_size; // 文件大小，字节为单位
    int m_addr[10]; // 文件的盘块的混合索引表
    int m_atime; // 最后访问时间
    int m_mtime; // 最后修改时间

    int m_number; // 外存对应的 inode 编号 

    // short m_dev; // 设备号
    // unsigned int m_flag; // 状态标志位
    int m_count; // 共享的 file 结构数
    int m_lastr; // 最后一次读的块号

public:
    MemINode();

    /**
     * 读取外存的 inode
     * 传入一个外存 inode 对象
     */
    void readDiskINode(DiskINode dinode);

    /**
     * 将内存 inode 节点转换成外存 inode 节点
     */
    DiskINode getDiskINode();
};

#endif