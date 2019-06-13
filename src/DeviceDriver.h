#include <fstream>
using namespace std;

#ifndef DEVICEDRIVER
#define DEVICEDRIVER

/**
 * 这个类封装了一些对于磁盘文件读写的操作
 */
class DeviceDriver {
private:
    static const char* DISK_FILE_NAME;
    fstream fs;

public: 
    DeviceDriver();
    ~DeviceDriver();

    /**
     * 检查虚拟磁盘文件是否存在
     */
    bool isExisting();

    /**
     * 将 buf 中的内容写入磁盘
     * offset 指字节数
     * 出错返回 false，否则返回 true
     */
    void write(const char* buf, const int buf_size, const int offset);

    /**
     * 将磁盘中的内容读入 buf
     * offset 指字节数
     * 出错返回 false，否则返回 true
     */
    void read(char* buf, const int buf_size, const int offset);
};

#endif