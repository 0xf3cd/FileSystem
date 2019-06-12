#include "INode.h"

DiskINode::DiskINode() {
    int i;

    d_mode = 0;
    d_nlink = 0;

    d_size = 0;
    for(i = 0; i < 10; i++) {
        d_addr[i] = 0;
    }
}

MemINode::MemINode() {
    m_mode = 0;
    m_nlink = 0;
    m_count = 0;
}

/**
 * 读取外存的 inode
 * 传入一个外存 inode 对象
 */
void MemINode::readDiskINode(DiskINode dinode) {
    int i;

    m_mode = dinode.d_mode; // 状态的标志位
    m_nlink = dinode.d_nlink; // 文件在目录中不同路径的数量
    m_uid = dinode.d_uid; // 文件所有者的用户标识数
    m_gid = dinode.d_gid;  // 文件所有者的组标识数
    m_size = dinode.d_size; // 文件大小，字节为单位
    
    // int d_addr[10]; // 文件的盘块的混合索引表
    for(i = 0; i < 10; i++) {
        m_addr[i] = dinode.d_addr[i];
    }

    m_atime = dinode.d_atime; // 最后访问时间
    m_mtime = dinode.d_mtime; // 最后修改时间

    m_dirty = 0;
    m_count = 0;
    // m_number
    // m_lastr
}

/**
 * 将内存 inode 节点转换成外存 inode 节点
 */
DiskINode MemINode::getDiskINode() {
    int i;
    DiskINode dinode;
    dinode.d_mode = m_mode; // 状态的标志位
    dinode.d_nlink = m_nlink; // 文件在目录中不同路径的数量
    dinode.d_uid = m_uid; // 文件所有者的用户标识数
    dinode.d_gid = m_gid;  // 文件所有者的组标识数
    dinode.d_size = m_size; // 文件大小，字节为单位
    for(i = 0; i < 10; i++) {
        dinode.d_addr[i] = m_addr[i];
    }
    dinode.d_atime = m_atime; // 最后访问时间
    dinode.d_mtime = m_mtime; // 最后修改时间

    return dinode;
}