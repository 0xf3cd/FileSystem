#include "DeviceDriver.h"
#include "INode.h"
#include "FileSystem.h"
#include "BufferManager.h"
#include "INodeManager.h"
#include "DirectoryEntry.h"
#include "File.h"
#include "FileManager.h"
#include "FileOperator.h"
#include "UserInterface.h"

#include <iostream>
#include <string>
#include <fstream>

DeviceDriver g_DeviceDriver;
SuperBlock g_SuperBlock;
BufferManager g_BufferManager;
FileSystem g_FileSystem;
INodeManager g_INodeManager;

const string text_dir = "./example/text.txt";
const string report_dir = "./example/report.pdf";
const string photo_dir = "./example/photo.jpg";

void initialize(FileOperator& FO);
void JerryTest(FileOperator& FO);

int main() {

    FileOperator FO;
    FO.format();

    FO.mkdir("bin");
    FO.mkdir("etc");
    FO.mkdir("home");
    FO.mkdir("dev");

    FO.cd("home");
    FO.mkdir("texts");
    FO.mkdir("reports");
    FO.mkdir("photos");

    initialize(FO); // 读入三个文件
    cout << "已将三个文件读入二级文件系统内" << endl;
    FO.goroot(); // 回到根目录

    JerryTest(FO); // 新建 test/Jerry 并进行文件读写测试
    FO.goroot(); // 回到根目录

    cout << endl;
    UserInterface UI(&FO);
    UI.start(); // 开始接收用户输入

    return 0;
}

void initialize(FileOperator& FO) {
    int i;
    ifstream fin;
    char buf[512];

    FO.cd("texts");
    fin.open(text_dir, ios::in | ios::binary);
    if(fin.is_open()) {
        fin.seekg(0, ios::end);
        int text_size = fin.tellg();
        fin.seekg(0, ios::beg);

        FO.fcreate("text.txt");
        FO.fopen("text.txt");

        const int full_blk_num = text_size / 512;
        const int rest_byte_num = text_size % 512;
        for(i = 0; i < full_blk_num; i++) {
            fin.read(buf, 512);
            FO.fwrite(buf, 512);
        }
        if(rest_byte_num) {
            fin.read(buf, rest_byte_num);
            FO.fwrite(buf, rest_byte_num);
        }

        FO.fclose();
        fin.close();
    } else {
        cout << "txt 文件无法打开，请检查路径" << endl;
    }
    FO.cd("..");

    FO.cd("reports");
    fin.open(text_dir, ios::in | ios::binary);
    if(fin.is_open()) {
        fin.seekg(0, ios::end);
        int text_size = fin.tellg();
        fin.seekg(0, ios::beg);

        FO.fcreate("reports.pdf");
        FO.fopen("reports.pdf");

        const int full_blk_num = text_size / 512;
        const int rest_byte_num = text_size % 512;
        for(i = 0; i < full_blk_num; i++) {
            fin.read(buf, 512);
            FO.fwrite(buf, 512);
        }
        if(rest_byte_num) {
            fin.read(buf, rest_byte_num);
            FO.fwrite(buf, rest_byte_num);
        }

        FO.fclose();
        fin.close();
    } else {
        cout << "pdf 文件无法打开，请检查路径" << endl;
    }
    FO.cd("..");

    FO.cd("photos");
    fin.open(text_dir, ios::in | ios::binary);
    if(fin.is_open()) {
        fin.seekg(0, ios::end);
        int text_size = fin.tellg();
        fin.seekg(0, ios::beg);

        FO.fcreate("photos.jpg");
        FO.fopen("photos.jpg");

        const int full_blk_num = text_size / 512;
        const int rest_byte_num = text_size % 512;
        for(i = 0; i < full_blk_num; i++) {
            fin.read(buf, 512);
            FO.fwrite(buf, 512);
        }
        if(rest_byte_num) {
            fin.read(buf, rest_byte_num);
            FO.fwrite(buf, rest_byte_num);
        }

        FO.fclose();
        fin.close();
    } else {
        cout << "jpg 文件无法打开，请检查路径" << endl;
    }
    FO.cd("..");
}

void JerryTest(FileOperator& FO) {
    char buf[800];
    char read_out[20];
    int i;
    for(i = 0; i < 800; i++) {
        buf[i] = 'a' + i % 26;
    }

    FO.mkdir("test");
    FO.cd("test");
    FO.fcreate("Jerry");
    FO.fopen("Jerry");
    cout << "/test/Jerry 创建成功" << endl; 

    FO.fwrite(buf, 800);
    cout << "向 Jerry 写入 800 字节" << endl; 

    FO.flseek(500); // 就读写指针定位到 500 字节
    FO.fread(read_out, 20);
    cout << "从 Jerry 500 字节开始读出 20 字节" << endl;
    FO.fclose();

    cout << "这 20 字节为：" << endl;
    for(i = 0; i < 20; i++) {
        cout << read_out[i];
    }
    cout << endl;
}