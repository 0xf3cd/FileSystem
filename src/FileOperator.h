#ifndef FILEOPERATOR
#define FILEOPERATOR

#include "FileSystem.h"
#include "FileManager.h"
#include <set>
#include <string.h>

/**
 * 直接向用户层提供创建、删除、写入文件/文件夹的方法
 */
class FileOperator {
private:
    FileSystem* FS;
    string cur_dir; // 当前的路径
    FileManager* cdir_fm; // 管理当前路径
    FileManager* ofile_fm; // 管理当前打开的文件

private:
    /**
     * 计算执行 cd come_to 后，路径变化情况
     */
    string getNewDir(string come_to);

public:
    FileOperator();
    ~FileOperator();

    /**
     * 当前是否打开文件
     */
    bool isOpeningFile() {
        return ofile_fm != nullptr;
    }

    /**
     * 格式化整个磁盘
     */
    void format();

    /**
     * 获取当前路径
     */
    string pwd();

    /**
     * 列出当前路径下所有文件和文件夹
     */
    set<string> ls();

    /**
     * 在当前路径下创建新的文件夹
     */
    int mkdir(string fld_name);

    /**
     * 在当前路径下创建新文件
     */
    int fcreate(string f_name);

    /**
     * 打开文件
     */
    int fopen(string f_name);

    /**
     * 关闭文件
     */
    void fclose();

    /**
     * 读取文件
     * 返回读取的字节数
     */
    int fread(char* buf, int length);

    /**
     * 写入文件
     * 返回写入的字节数
     */
    int fwrite(char* buf, int length);

    /**
     * 更改文件读写的 offset
     */
    void flseek(int noffset);

    /**
     * 返回当前 offset
     */
    int curseek();

    /**
     * 查看文件大小
     */
    int fsize();

    /**
     * 删除文件
     */
    int fdelete(string f_name);

    /**
     * 删除目录（递归）
     */
    int dirdelete(string fld_name);

    /**
     * 更换路径
     */
    int cd(string fld_name="");

    /**
     * 复制
     */
    int cp(string f_name, string des_name);

    /**
     * 移动
     */
    int mv(string f_name, string des_name);

    /**
     * 回到根目录
     */
    void goroot();

    /**
     * 判断某个文件是否是文件夹
     */
    // bool isFolder(string fld_name);
};

#endif