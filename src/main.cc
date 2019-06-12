#include "DeviceDriver.h"
#include "INode.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "DirectoryEntry.h"

#include <iostream>
#include <string.h>

DeviceDriver g_DeviceDriver;
SuperBlock g_SuperBlock;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
INodeManager g_INodeManager;

int main() {
    // g_FileSystem.formatDisk();

    // FS.allocDiskINode();
    // FS.freeDiskINode(0);

    // FS.allocDiskINode();
    // FS.allocDiskINode();
    // FS.allocDiskINode();
    // FS.freeDiskINode(1);

    // FS.allocDiskINode();

    // for(int i = 0; i < 300; i++) {
    //     // g_BufferManager.freeBuf(FS.allocBlock());
    //     // g_FileSystem.allocDiskINode();
    //     g_INodeManager.getNewMINode();
    // }

    // cout << endl;
    // auto p1 = g_INodeManager.getLoadedDINode(13);
    // g_INodeManager.writeBackMINode(p1);
    // g_INodeManager.freeMINode(p1);
    // // cout << p1 -> m_count << endl;

    // auto p2 = g_INodeManager.getLoadedDINode(15);
    // g_INodeManager.writeBackMINode(p2);
    // g_INodeManager.freeMINode(p2);
    // // cout << p2 -> m_count << endl;

    // auto p3 = g_INodeManager.readDINode(20);
    // g_INodeManager.writeBackMINode(p3);
    // g_INodeManager.freeMINode(p3);
    // // cout << p3 -> m_count << endl;

    auto proot = g_INodeManager.readDINode(0);
    cout << proot -> m_size << endl;
    cout << proot -> m_addr[0] << endl;
    g_INodeManager.writeBackMINode(proot);

    auto rootblk = g_BufferManager.readBuf(proot -> m_addr[0]);
    DirectoryEntry dir[2];
    memcpy((char*)dir, rootblk -> b_addr, 64);
    cout << dir[0].fname << ' ' << dir[0].ino << endl;
    cout << dir[1].fname << ' ' << dir[1].ino << endl;

    return 0;
}