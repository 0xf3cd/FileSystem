#include "DeviceDriver.h"
#include "INode.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "DirectoryEntry.h"
#include "File.h"
#include "FileManager.h"

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
    
    FileManager FM;

    cout << FM.createFile("temp1") << endl;
    cout << FM.createFile("folder", true) << endl;
    cout << FM.createFile("temp2") << endl;

    FM.renameFile("temp2", "temp2_");
    FMAP* item_map = FM.loadItems();
    FMAP::iterator it;
    for(it = item_map -> begin(); it != item_map -> end(); it++) {
        cout << it -> first << ", " << it -> second << endl;
    }
    cout << item_map -> size() << endl;
    cout << endl;
    delete item_map;

    FileManager FM2(2);
    FM.copyFile("temp2_", &FM2);
    cout << FM2.createFile("temp11") << endl;
    cout << FM2.createFile("folder1", true) << endl;
    cout << FM2.createFile("temp21") << endl;

    FMAP* item_map2 = FM2.loadItems();
    for(it = item_map2 -> begin(); it != item_map2 -> end(); it++) {
        cout << it -> first << ", " << it -> second << endl;
    }
    cout << item_map2 -> size() << endl;
    cout << endl;
    delete item_map2;
}