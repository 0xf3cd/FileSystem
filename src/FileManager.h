#ifndef FILEMANAGER
#define FILEMANAGER

#include "BufferManager.h"
#include "INodeManager.h"
#include "File.h"
#include "DirectoryEntry.h"

#define ROOT_INO 0 // 根目录的 inode 编号

/**
 * 通过这个类可以管理 File 类的对象
 * 这个类提供更便捷的方法操作文件夹目录项（如增删目录项，为目录项改名等）
 * 也提供便捷的方法操纵文件
 */
class FileManager {
    
};

#endif