#ifndef FILEMANAGER
#define FILEMANAGER

#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "File.h"
#include "DirectoryEntry.h"
#include <string.h>
#include <map>
using namespace std;

#define ROOT_INO 0 // 根目录的 inode 编号
typedef map<string, int> FMAP; // 文件名到外存 inode 号的映射

/**
 * 通过这个类可以管理 File 类的对象
 * 这个类提供更便捷的方法操作文件夹目录项（如增删目录项，为目录项改名等）
 * 也提供便捷的方法操纵文件
 */
class FileManager {
private:
    FileSystem* FS; 
    BufferManager* BM;
    INodeManager* IM; // 用于申请新的内存快
    File* file;

private:
    /**
     * 根据 inode 号找到并打开文件
     */
    File* openFile(int ino);

    /**
     * 删除当前目录下的某个文件项
     */
    void deleteItem(string fname);

public:
    FileManager(); // 默认打开根目录
    FileManager(int ino); // 打开 ino 对应的 inode 块对应的文件
    ~FileManager();

    /**
     * 检查文件是否为文件夹
     */
    bool isFolder();

    /**
     * 得到文件大小
     */
    int getSize();

    /**
     * 当前文件夹下是否有某名称的文件
     * ！调用前需检查当前文件是否为文件夹！
     */
    bool hasItem(string fname);

    /**
     * 统计文件夹中的目录项数量
     * ！调用前需检查当前文件是否为文件夹！
     */
    int countItems();

    /**
     * 加载文件夹中的目录项名称及对应 inode 号码
     * ！调用前需检查当前文件是否为文件夹！
     */
    FMAP* loadItems();

    /**
     * 如果文件夹下有某名称的文件，则返回它的外存 inode 编号
     * ！调用前需检查当前文件是否为文件夹！
     */
    int getDiskINodeNo(string fname);

    /**
     * 在当前目录下新增文件
     * 默认生成普通文件
     * 返回外存 inode 编号
     * ！调用前需检查当前文件是否为文件夹！
     */
    int createFile(string nfname, bool is_folder=false);

    /**
     * 在当前目录下删除普通文件
     * 不能删除文件夹！
     * ！调用前需检查当前文件是否为文件夹！
     */
    void deleteNormalFile(string fname);

    /**
     * 在当前目录下删除文件夹
     * 默认文件夹为空才能删除，否则不能删除
     * 传入 true 则先递归删除文件夹中所有内容再删除本身
     * ！调用前需检查当前文件是否为文件夹！
     */
    void deleteFolder(string fname, bool force_del=false);

    /**
     * 重命名当前目录下某文件
     * oname: old name, nname: new name
     * ！调用前需检查当前文件是否为文件夹！
     */
    void renameFile(string oname, string nname);

    /**
     * 复制一个文件到新的地址
     * 第一个参数是当前目录下某文件的名字，第二个参数是指向目的地址的 FileManager 指针
     * 本质是做了勾连，共享一个外存 inode
     * ！调用前需检查当前文件是否为文件夹！
     */
    void copyFile(string fname, FileManager* des_f);

    /**
     * 移动一个文件到新的地址
     * 第一个参数是当前目录下某文件的名字，第二个参数是指向目的地址的 FileManager 指针
     * ！调用前需检查当前文件是否为文件夹！
     */
    void moveFile(string fname, FileManager* des_f);

    /**
     * 传入相对地址，返回对应文件的 inode 编号
     * ！调用前需检查当前文件是否为文件夹！
     */
    int getFileIno(string addr);

    /**
     * 得到文件读写指针地址
     */
    int getFOffset();

    /**
     * 更改文件读写指针地址
     */
    void setFOffset(int noffset);

    /**
     * 读文件
     * 以当前文件读写指针为起始
     * 返回读取的字节数
     * !调用前需要检查当前文件是否为普通文件！
     */
    int read(char* content, int length);

    /**
     * 写文件
     * 以当前文件读写指针为起始
     * 返回写入的字节数
     * !调用前需要检查当前文件是否为普通文件！
     */
    int write(char* content, int length);

    /**
     * 删除文件的一部分
     * 以当前文件读写指针为起始
     * 删除的内容保存在 content 中（如果 content 不为 nullptr）
     * content 为 nullptr 则不保存删除的内容
     * 返回删除的字节数
     * !调用前需要检查当前文件是否为普通文件！
     */
    int remove(char* content, int length);
};

#endif