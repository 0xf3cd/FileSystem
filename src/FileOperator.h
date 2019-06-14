#ifndef FILEOPERATOR
#define FILEOPERATOR

#include "FileManager.h"
#include <map>

typedef map<int, FileManager*> FMMAP;

/**
 * 直接向用户层提供创建、删除、写入文件/文件夹的方法
 */
class FileOperator {

public:
    FileOperator();
    ~FileOperator();
};

#endif