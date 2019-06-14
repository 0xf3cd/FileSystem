#include "FileOperator.h"
#include "FileManager.h"
#include "FileSystem.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

extern FileSystem g_FileSystem;
extern vector<string> split(const string&, const string&);

FileOperator::FileOperator() {
    FS = &g_FileSystem;
    cur_dir = "/";
    cdir_fm = new FileManager;
    ofile_fm = nullptr;
}

FileOperator::~FileOperator() {
    if(cdir_fm != nullptr) {
        delete cdir_fm;
    }

    if(ofile_fm != nullptr) {
        delete ofile_fm;
    }
}

/**
 * 格式化整个磁盘
 */
void FileOperator::format() {
    if(cdir_fm != nullptr) {
        delete cdir_fm;
    }

    if(ofile_fm != nullptr) {
        delete ofile_fm;
    }

    FS -> formatDisk();
    cur_dir = "/";
    cdir_fm = new FileManager;
    ofile_fm = nullptr;
}

/**
 * 获取当前路径
 */
string FileOperator::pwd() {
    return cur_dir;
}

/**
 * 列出当前路径下所有文件和文件夹
 */
set<string> FileOperator::ls() {
    FMAP* m = cdir_fm -> loadItems();
    FMAP::iterator it;
    set<string> s;
    for(it = m -> begin(); it != m -> end(); it++) {
        // cout << it -> first << endl;
        s.insert(it -> first);
    }
    delete m;
    return s;
}

/**
 * 在当前路径下创建新的文件夹
 */
int FileOperator::mkdir(string fld_name) {
    if(cdir_fm -> hasItem(fld_name)) {
        return -1; // 表示已经存在同名文件
    }

    cdir_fm -> createFile(fld_name, true);
    return 0;
}

/**
 * 在当前路径下创建新文件
 */
int FileOperator::fcreate(string f_name) {
    if(cdir_fm -> hasItem(f_name)) {
        return -1; // 表示已经存在同名文件
    }

    cdir_fm -> createFile(f_name, false);
    return 0;
}

/**
 * 打开文件
 */
int FileOperator::fopen(string f_name) {
    if(ofile_fm != nullptr) {
        delete ofile_fm;
    }
     
    if(!cdir_fm -> hasItem(f_name)) {
        return -1; // 不存在这个文件
    }

    const int ino = cdir_fm -> getDiskINodeNo(f_name);
    ofile_fm = new FileManager(ino);
    if(ofile_fm -> isFolder()) {
        delete ofile_fm;
        ofile_fm = nullptr;
        return -2; // 打开的是文件夹
    }

    // 打开的是普通文件
    return 0;
}

/**
 * 关闭文件
 */
void FileOperator::fclose() {
    if(ofile_fm != nullptr) {
        delete ofile_fm;
    }
    ofile_fm = nullptr;
}

/**
 * 读取文件
 * 返回读取的字节数
 */
int FileOperator::fread(char* buf, int length) {
    if(ofile_fm == nullptr) {
        return 0; // 文件未打开
    }

    return ofile_fm -> read(buf, length);
}

/**
 * 写入文件
 * 返回写入的字节数
 */
int FileOperator::fwrite(char* buf, int length) {
    if(ofile_fm == nullptr) {
        return 0; // 文件未打开
    }

    return ofile_fm -> write(buf, length);
}

/**
 * 更改文件读写的 offset
 */
void FileOperator::flseek(int noffset) {
    if(ofile_fm == nullptr) {
        return; // 文件未打开
    }

    ofile_fm -> setFOffset(noffset);
}

/**
 * 返回当前 offset
 */
int FileOperator::curseek() {
    if(ofile_fm == nullptr) {
        return -1; // 文件未打开
    }

    return ofile_fm -> getFOffset();
}

/**
 * 查看文件大小
 */
int FileOperator::fsize() {
    if(ofile_fm == nullptr) {
        return -1; // 文件未打开
    }

    return ofile_fm -> getSize();
}

/**
 * 删除文件
 */
int FileOperator::fdelete(string f_name) {
    if(!cdir_fm -> hasItem(f_name)) {
        return -1; // 不存在这个文件
    }

    const int ino = cdir_fm -> getDiskINodeNo(f_name);
    FileManager* temp_fm = new FileManager(ino);
    if(temp_fm -> isFolder()) {
        delete temp_fm;
        return -2; // 要删除的是文件夹
    }

    // 打开的是普通文件
    delete temp_fm;
    cdir_fm -> deleteNormalFile(f_name);
    return 0;
}

/**
 * 删除目录（递归）
 */
int FileOperator::dirdelete(string fld_name) {
    if(!cdir_fm -> hasItem(fld_name)) {
        return -1; // 不存在这个文件
    }

    const int ino = cdir_fm -> getDiskINodeNo(fld_name);
    FileManager* temp_fm = new FileManager(ino);
    if(!temp_fm -> isFolder()) {
        delete temp_fm;
        return -2; // 要删除的是普通文件
    }

    // 打开的是文件夹
    delete temp_fm;
    cdir_fm -> deleteFolder(fld_name, true);
    return 0;
}

/**
 * 回到根目录
 */
void FileOperator::goroot() { 
    if(cdir_fm != nullptr) {
        delete cdir_fm;
    }

    if(ofile_fm != nullptr) {
        delete ofile_fm;
    }

    cur_dir = "/";
    cdir_fm = new FileManager;
    ofile_fm = nullptr;

    // cout << cdir_fm -> getFileIno("Jerry");
}

/**
 * 更换路径
 */
int FileOperator::cd(string fld_name) {
    if(fld_name == "" || fld_name == "/") {
        cur_dir = "/";
        delete cdir_fm;
        cdir_fm = new FileManager(ROOT_INO);
        return 0;
    }

    const int ino = cdir_fm -> getFileIno(fld_name);
    if(ino == -1) {
        return -1; // 不存在这个文件
    }

    FileManager* temp_fm = new FileManager(ino);
    if(!temp_fm -> isFolder()) {
        delete temp_fm;
        return -2; // 要进入的是普通文件
    }
    delete temp_fm;
    string new_dir = getNewDir(fld_name);
    cur_dir = new_dir;
    delete cdir_fm;
    cdir_fm = new FileManager(ino);
    return 0;
}

/**
 * 计算执行 cd come_to 后，路径变化情况
 */
string FileOperator::getNewDir(string come_to) {
    vector<string> sv = split(come_to, "/");
    for(vector<string>::const_iterator it = sv.begin(); it != sv.end(); it++) {
        const string tmp = *it;
        if(tmp == ".") {
            // return cur_dir;
            continue;
        }

        if(tmp == "..") {
            if(cur_dir == "/") {
                // return "/";
                continue;
            }

            int count = 0;
            int length = cur_dir.length();
            int i;
            for(i = 0; i < length; i++) {
                if(cur_dir[i] == '/') {
                    count++;
                }
            }

            string to_return = "";
            int met_count = 0;
            i = 0;
            while(true) {
                if(cur_dir[i] == '/') {
                    met_count++;
                }
                if(met_count == count) {
                    break;
                }

                to_return += cur_dir[i];
                i++;
            }
            cur_dir = to_return;
            continue;
        }

        cur_dir += (cur_dir == "/"? "": "/") + tmp;
    }

    if(cur_dir == "") {
        cur_dir = "/";
    }
    return cur_dir;
}

/**
 * 复制
 */
int FileOperator::cp(string f_name, string des_name) {
    const int from_ino = cdir_fm -> getFileIno(f_name);
    const int to_ino = cdir_fm -> getFileIno(des_name);
    if(from_ino == -1) {
        return -1; // 不存在 f_name
    }
    if(to_ino == -1) {
        return -2; // 不存在 des_name
    }

    FileManager fromFM(from_ino);
    FileManager toFM(to_ino);
    if(!toFM.isFolder()) {
        return -3; // des_name 不是文件夹
    }

    fromFM.copyFile(".", &toFM);
    return 0;
}

/**
 * 移动
 */
int FileOperator::mv(string f_name, string des_name) {
    const int from_ino = cdir_fm -> getFileIno(f_name);
    const int to_ino = cdir_fm -> getFileIno(des_name);
    if(from_ino == -1) {
        return -1; // 不存在 f_name
    }
    if(to_ino == -1) {
        return -2; // 不存在 des_name
    }

    FileManager fromFM(from_ino);
    FileManager toFM(to_ino);
    if(!toFM.isFolder()) {
        return -3; // des_name 不是文件夹
    }

    fromFM.moveFile(".", &toFM);
    return 0;
}