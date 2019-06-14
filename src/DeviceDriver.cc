#include "DeviceDriver.h"

const char* DeviceDriver::DISK_FILE_NAME = "./disk.img";

DeviceDriver::DeviceDriver() {
    fs.open(DISK_FILE_NAME, ios::binary | ios::out | ios::in);
}

DeviceDriver::~DeviceDriver() {
    fs.close();
}

/**
 * 检查虚拟磁盘文件是否存在
 */
bool DeviceDriver::isExisting() {
    return fs.is_open();
}

/**
 * 将 buf 中的内容写入磁盘
 * offset 指字节数
 */
void DeviceDriver::write(const char* buf, const int buf_size, const int offset) {
    fs.seekp(offset, ios::beg);
	fs.write(buf, buf_size);
}

/**
 * 将磁盘中的内容读入 buf
 * offset 指字节数
 */
void DeviceDriver::read(char* buf, const int buf_size, const int offset) {
    fs.seekg(offset, ios::beg);
    fs.read(buf, buf_size);
}