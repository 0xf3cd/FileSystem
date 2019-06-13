#include "DeviceDriver.h"
#include "INode.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "DirectoryEntry.h"
#include "File.h"

#include <iostream>
#include <string.h>

DeviceDriver g_DeviceDriver;
SuperBlock g_SuperBlock;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
INodeManager g_INodeManager;

int main() {
    g_FileSystem.formatDisk();

    MemINode* proot = g_INodeManager.readDINode(0);
    g_INodeManager.writeBackMINode(proot);

    // Buffer* rootblk = g_BufferManager.readBuf(proot -> m_addr[0]);
    // DirectoryEntry dir[2];
    // memcpy((char*)dir, rootblk -> b_addr, 64);
    // cout << dir[0].fname << ' ' << dir[0].ino << endl;
    // cout << dir[1].fname << ' ' << dir[1].ino << endl;

    MemINode* newfile = g_INodeManager.getNewMINode();
    File F(newfile);
    // proot -> m_mode |= MemINode::IUPD;
    // for(int i = 0; i < 6 + 128*2 +128*128*2 - 1 ; i++) {
    //     Buffer* temp = F.applyNewBlk();
    //     proot -> m_size += 512; // 模拟写入 512 字节
    //     g_BufferManager.writeBuf(temp);
    // }

    // for(int i = 0; i < 6 + 128*2 + 128*128*2; i++) {
    //     cout << F.mapBlk(i) << endl;
    // }

    cout << F.getFileSize() << endl;
    // char tc[10000];
    // cout << F.f_offset << endl;
    // cout << F.read(tc, 32) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;
    // cout << F.read(tc, 64) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;
    // cout << F.read(tc, 500) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;
    // cout << F.read(tc, 946) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;
    // cout << F.read(tc, 4615) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;
    // cout << F.read(tc, 2) << endl;
    // cout << endl;

    // cout << F.f_offset << endl;

    // char tc[64];
    // tc[0] = 'a';
    // tc[1] = '*';
    // tc[2] = 'z';
    // tc[3] = '.';
    // cout << F.write(tc, 4) << endl;

    char tc[10000];
    char tc_[10000];

    for(int i = 0; i < 10000; i++) {
        tc[i] = 'a' + i % 26;
    }
    // for(int i = 0; i < 10000; i++) {
    //     cout << tc[i] << endl;
    // }

    F.f_offset = 0;
    cout << F.write(tc, 10000) << endl;
    // cout << F.getFileSize() << endl;

    F.f_offset = 0;
    cout << F.read(tc_, 10000) << endl;
    for(int i = 0; i < 10000; i++) {
        cout << tc_[i] << endl;
    }
    
    // cout << F.getBlkNum() << endl;

    // auto x = g_BufferManager.readBuf(211);
    // for(int i = 0; i < 512; i++) {
    //     cout << (x -> b_addr)[i] << endl;
    // }
    return 0;
}