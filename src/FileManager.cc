#include "FileManager.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "File.h"
#include "DirectoryEntry.h"
#include <map>
#include <iostream> 
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

extern FileSystem g_FileSystem;
extern BufferManager g_BufferManager;
extern INodeManager g_INodeManager;

vector<string> split(const string &s, const string &seperator){
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;
    
    while(i != s.size()){
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while(i != s.size() && flag == 0){
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
            if(s[i] == seperator[x]) {
                ++i;
                flag = 0;
                break;
            }
        }
        
        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0){
            for(string_size x = 0; x < seperator.size(); ++x)
            if(s[j] == seperator[x]){
                flag = 1;
                break;
            }
            if(flag == 0) 
                ++j;
        }
        if(i != j){
            result.push_back(s.substr(i, j-i));
            i = j;
        }
    }
    return result;
}


/**
 * 根据 inode 号找到并打开文件
 */
File* FileManager::openFile(int ino) {
    MemINode* minode = IM -> readDINode(ino);
    File* f = new File(minode);
    return f;
}

FileManager::FileManager() { // 默认打开根目录
    FS = &g_FileSystem;
    BM = &g_BufferManager;
    IM = &g_INodeManager;
    int ino = ROOT_INO;
    file = openFile(ino);
}

FileManager::FileManager(int ino) { // 打开 ino 对应的 inode 块对应的文件
    FS = &g_FileSystem;
    BM = &g_BufferManager;
    IM = &g_INodeManager;
    file = openFile(ino);
}

FileManager::~FileManager() {
    MemINode* minode = file -> f_minode;
    // if(IM -> hasLoadedDINode(minode -> m_number)) {
    //     IM -> freeMINode(minode); // 释放构造函数中申请的内存 inode 节点
    // }
    delete file;
}

/**
 * 检查文件是否为文件夹
 */
bool FileManager::isFolder() {
    int f_type = file -> f_type;
    return (f_type & File::FDIR) != 0;
}

/**
 * 得到文件大小
 */
int FileManager::getSize() {
    return file -> getFileSize();
}

/**
 * 当前文件夹下是否有某名称的文件
 * ！调用前需检查当前文件是否为文件夹！
 */
bool FileManager::hasItem(string fname) {
    int i;
    const int item_num = countItems();
    const int f_size = getSize();
    char* read_buf = new char[f_size];
    file -> f_offset = 0;
    file -> read(read_buf, f_size);

    DirectoryEntry* items = (DirectoryEntry*)(read_buf);
    for(i = 0; i < item_num; i++) {
        if(items[i].fname == fname) {
            delete[] read_buf;
            return true;
        }
    }
    delete[] read_buf;
    return false;
}

/**
 * 统计文件夹中的目录项数量
 * ！调用前需检查当前文件是否为文件夹！
 */
int FileManager::countItems() {
    const int f_size = getSize();
    return f_size / sizeof(DirectoryEntry);
}

/**
 * 加载文件夹中的目录项名称及对应 inode 号码
 * ！调用前需检查当前文件是否为文件夹！
 */
FMAP* FileManager::loadItems() {
    int i;
    const int item_num = countItems();
    const int f_size = getSize();
    char* read_buf = new char[f_size];
    file -> f_offset = 0;
    file -> read(read_buf, f_size);

    FMAP* fm = new FMAP();
    DirectoryEntry* items = (DirectoryEntry*)(read_buf);
    for(i = 0; i < item_num; i++) {
        string fname = items[i].fname;
        (*fm)[fname] = items[i].ino;
    }
    return fm;
}

/**
 * 删除当前目录下的某个文件项
 */
void FileManager::deleteItem(string fname) {
    int i;
    const int item_num = countItems();
    const int f_size = getSize();
    char* read_buf = new char[f_size];
    file -> f_offset = 0;
    file -> read(read_buf, f_size);

    int index;
    DirectoryEntry* items = (DirectoryEntry*)(read_buf);
    for(i = 0; i < item_num; i++) {
        if(items[i].fname == fname) {
            index = i;
            break;
        }
    }

    if(i == item_num) {
        cout << "没有找到目录中的 " << fname << endl;
        return; 
    }

    file -> f_offset = sizeof(DirectoryEntry) * index;
    file -> remove(nullptr, sizeof(DirectoryEntry));
}

/**
 * 如果文件夹下有某名称的文件，则返回它的外存 inode 编号
 * ！调用前需检查当前文件是否为文件夹！
 */
int FileManager::getDiskINodeNo(string fname) {
    int i;
    const int item_num = countItems();
    const int f_size = getSize();
    char* read_buf = new char[f_size];
    file -> f_offset = 0;
    file -> read(read_buf, f_size);

    DirectoryEntry* items = (DirectoryEntry*)(read_buf);
    for(i = 0; i < item_num; i++) {
        if(items[i].fname == fname) {
            return items[i].ino;
        }
    }
    return -1;
}

/**
 * 在当前目录下新增文件
 * 默认生成普通文件
 * 返回外存 inode 
 * ！调用前需检查当前文件是否为文件夹！
 */
int FileManager::createFile(string nfname, bool is_folder) {
    if(nfname.length() > 27) {
        cout << "文件名 " << nfname << " 过长" << endl;
        return -1; 
    }

    if(hasItem(nfname)) {
        cout << "文件 " << nfname << " 已存在" << endl;
        return -1;
    }

    MemINode* new_minode = IM -> getNewMINode();
    const int dino = new_minode -> m_number; // 在外存 inode 的编号
    if(is_folder) {
        new_minode -> m_mode |= MemINode::IFDIR; // 设置成文件夹
        File f(new_minode);
        // 为新的文件夹新建 . .. 两项
        DirectoryEntry dir[2];
        memcpy(dir[0].fname, ".", 2);
        memcpy(dir[1].fname, "..", 3);
        dir[0].ino = dino;
        dir[1].ino = file -> f_minode -> m_number;
        f.f_offset = 0;
        f.write((char*)dir, sizeof(dir));
    }
    IM -> freeMINode(new_minode);

    DirectoryEntry new_item;
    new_item.ino = dino;
    int i;
    for(i = 0; i < nfname.length(); i++) {
        new_item.fname[i] = nfname[i];
    }
    new_item.fname[i] = '\0'; // 添加尾 0

    file -> f_offset = getSize();
    file -> write((char*)&new_item, sizeof(DirectoryEntry));
    return dino;
}

/**
 * 在当前目录下删除普通文件
 * 不能删除文件夹！
 * ！调用前需检查当前文件是否为文件夹！
 */
void FileManager::deleteNormalFile(string fname) {
    if(!hasItem(fname)) {
        cout << "当前目录下没有 " << fname << " 文件" << endl;
        return;
    }

    const int dino = getDiskINodeNo(fname); // 本文件的外存 inode 节点编号
    MemINode* minode = IM -> readDINode(dino);
    if(minode -> m_mode & MemINode::IFDIR) {
        // 如果要删除的文件是一个文件夹，则结束
        IM -> freeMINode(minode);
        cout << fname << " 是一个文件夹，不是文件" << endl;
        return;
    }

    deleteItem(fname); // 删除目录项
    File* f = new File(minode);
    f -> deleteAll(); // 删除文件所有内容
    delete f;
    if(minode -> m_nlink > 1) {
        // 如果有勾连的情况，则不释放外存 inode
        minode -> m_nlink -= 1;
        minode -> m_mode |= MemINode::IUPD;
        IM -> freeMINode(minode);
        return;
    }
    // 否则需要释放外存 inode
    // cout << "!" << dino << endl;
    IM -> freeMINode(minode);
    FS -> freeDiskINode(dino); // 释放外存的 inode
}

/**
 * 在当前目录下删除文件夹
 * 默认文件夹为空才能删除，否则不能删除
 * 传入 true 则先递归删除文件夹中所有内容再删除本身
 * ！调用前需检查当前文件是否为文件夹！
 */
void FileManager::deleteFolder(string fname, bool force_del) {
    const int dino = getDiskINodeNo(fname); // 本文件的外存 inode 节点编号
    MemINode* minode = IM -> readDINode(dino);
    if(!(minode -> m_mode & MemINode::IFDIR)) {
        // 如果是普通文件，则结束
        IM -> freeMINode(minode);
        cout << fname << " 是一个普通文件，不是文件夹" << endl;
        return;
    }

    if(minode -> m_nlink > 1) { // 如果这个文件夹也被其他文件共享。则只需要将勾连数减一，并在当前目录下移除即可
        minode -> m_nlink -= 1;
        deleteItem(fname);
        minode -> m_mode |= MemINode::IUPD;
        IM -> freeMINode(minode);
        return;
    } else {
        FileManager fm(dino); // 子文件夹的 FileManager
        FMAP* item_map = fm.loadItems(); // 统计子文件下所有的文件情况

        if(item_map -> size() == 2) { // 仅仅包含 . .. 两项
            // 如果 fname 这个文件夹中没有其他文件
            // 可以直接删除
            deleteItem(fname);
            FS -> freeDiskINode(dino); // 释放外存的 inode
            IM -> freeMINode(minode);
            delete item_map; 
            return;
        }
        // 否则 fname 不为空，其中还有其他文件

        if(!force_del) {
            // 如果没有设定强制删除，则结束整个过程
            cout << fname << " 文件夹不为空，不能删除" << endl;    
            IM -> freeMINode(minode);
            delete item_map; 
            return;
        }

        FMAP::iterator it;
        for(it = item_map -> begin(); it != item_map -> end(); it++) {
            const string subf_name = it -> first;
            const int subf_ino = it -> second;

            if(subf_name == "." || subf_name == "..") {
                continue;
            }
            MemINode* subf_minode = IM -> readDINode(subf_ino);
            if(subf_minode -> m_mode & MemINode::IFDIR) {
                // 如果遇到了一个文件夹
                cout << "删除了文件夹 " << subf_name << endl;
                fm.deleteFolder(subf_name, true); // 使用递归删除文件
            } else {
                // 如果是普通文件
                cout << "删除了文件 " << subf_name << endl;
                fm.deleteNormalFile(subf_name);
            }
        }
        delete item_map; 
        deleteItem(fname);
    }
    
    IM -> freeMINode(minode);
}

/**
 * 重命名当前目录下某文件
 * oname: old name, nname: new name
 * ！调用前需检查当前文件是否为文件夹！
 */
void FileManager::renameFile(string oname, string nname) {
    if(!hasItem(oname)) {
        cout << "没有文件 " << oname << endl;
        return; 
    }

    if(nname.length() > 27) {
        cout << "文件名 " << nname << " 过长" << endl;
        return; 
    }

    int i;
    const int item_num = countItems();
    const int f_size = getSize();
    char* read_buf = new char[f_size];
    file -> f_offset = 0;
    file -> read(read_buf, f_size);

    int index;
    DirectoryEntry* items = (DirectoryEntry*)(read_buf);
    for(i = 0; i < item_num; i++) {
        if(items[i].fname == oname) {
            index = i;
            break;
        }
    }
    if(i == item_num) {
        cout << "没有找到目录中的 " << oname << endl;
        return; 
    }

    DirectoryEntry new_dir;
    file -> f_offset = sizeof(DirectoryEntry) * index;
    file -> read((char*)&new_dir, sizeof(DirectoryEntry));
    for(i = 0; i < nname.length(); i++) {
        new_dir.fname[i] = nname[i];
    }
    new_dir.fname[i] = '\0';

    file -> f_offset = sizeof(DirectoryEntry) * index;
    file -> write((char*)&new_dir, sizeof(DirectoryEntry));
}

/**
 * 复制一个文件到新的地址
 * 第一个参数是当前目录下某文件的名字，第二个参数是指向目的地址的 FileManager 指针
 * 本质是做了勾连，共享一个外存 inode
 * ！调用前需检查当前文件是否为文件夹！
 */
void FileManager::copyFile(string fname, FileManager* des_f) {
    if(!hasItem(fname)) {
        cout << "没有文件 " << fname << endl;
        return; 
    }
    if(!des_f -> isFolder()) {
        cout << "目标地址不是一个文件夹！" << endl;
        return;
    }

    const int dino = getDiskINodeNo(fname);
    DirectoryEntry new_item;
    new_item.ino = dino;
    int i;
    for(i = 0; i < fname.length(); i++) {
        new_item.fname[i] = fname[i];
    }
    new_item.fname[i] = '\0'; // 添加尾 0

    // 将 inode 进行共享，写入目标文件目录
    (des_f -> file) -> f_offset = des_f -> getSize();
    (des_f -> file) -> write((char*)&new_item, sizeof(DirectoryEntry)); 

    MemINode* minode = IM -> readDINode(dino);
    minode -> m_nlink += 1; // 增加勾连数
    minode -> m_mode |= MemINode::IUPD;
    IM -> freeMINode(minode);
}

/**
 * 移动一个文件到新的地址
 * 第一个参数是当前目录下某文件的名字，第二个参数是指向目的地址的 FileManager 指针
 * ！调用前需检查当前文件是否为文件夹！
 */
void FileManager::moveFile(string fname, FileManager* des_f) {
    if(!hasItem(fname)) {
        cout << "没有文件 " << fname << endl;
        return; 
    }
    if(!des_f -> isFolder()) {
        cout << "目标地址不是一个文件夹！" << endl;
        return;
    }

    const int dino = getDiskINodeNo(fname);
    DirectoryEntry new_item;
    new_item.ino = dino;
    int i;
    for(i = 0; i < fname.length(); i++) {
        new_item.fname[i] = fname[i];
    }
    new_item.fname[i] = '\0'; // 添加尾 0

    // 将 inode 进行共享，写入目标文件目录
    (des_f -> file) -> f_offset = des_f -> getSize();
    (des_f -> file) -> write((char*)&new_item, sizeof(DirectoryEntry)); 

    deleteItem(fname);
}

/**
 * 传入相对地址，返回对应文件的 inode 编号
 * ！调用前需检查当前文件是否为文件夹！
 */
int FileManager::getFileIno(string addr) {
    string s(addr);
    vector<string> sv;
    vector<string>::const_iterator it;

    int cur_ino = (file -> f_minode) -> m_number;
    // cout << cur_ino << "!!!" << (file -> f_minode) -> m_number << endl;

    sv = split(s, "/");
    for(it = sv.begin(); it != sv.end(); it++) {
        FileManager tFM(cur_ino);
        
        if(tFM.hasItem(*it) && tFM.isFolder()) {
            cur_ino = tFM.getDiskINodeNo(*it);
            // cout << cur_ino << "!!!" << (file -> f_minode) -> m_number << endl;
        } else {
            return -1;
        }
    }
    // cout << cur_ino << "!!!" << (file -> f_minode) -> m_number << endl;
    return cur_ino;
}

/**
 * 得到文件读写指针地址
 */
int FileManager::getFOffset() {
    return file -> f_offset;
}

/**
 * 更改文件读写指针地址
 */
void FileManager::setFOffset(int noffset) {
    file -> f_offset = noffset;
}

/**
 * 读文件
 * 以当前文件读写指针为起始
 * 返回读取的字节数
 * !调用前需要检查当前文件是否为普通文件！
 */
int FileManager::read(char* content, int length) {
    return file -> read(content, length);
}

/**
 * 写文件
 * 以当前文件读写指针为起始
 * 返回写入的字节数
 * !调用前需要检查当前文件是否为普通文件！
 */
int FileManager::write(char* content, int length) {
    return file -> write(content, length);
    // cout << "!!!!" << endl;
}

/**
 * 删除文件的一部分
 * 以当前文件读写指针为起始
 * 删除的内容保存在 content 中（如果 content 不为 nullptr）
 * content 为 nullptr 则不保存删除的内容
 * 返回删除的字节数
 * !调用前需要检查当前文件是否为普通文件！
 */
int FileManager::remove(char* content, int length) {
    return file -> remove(content, length);
}
