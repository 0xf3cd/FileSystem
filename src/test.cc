#ifndef DEVICEDRIVER
#define DEVICEDRIVER
#include "./DeviceDriver.h"
#endif

#ifndef DISKINODE
#define DISKINODE
#include "./DiskINode.h"
#endif

#ifndef FILESYSTEM
#define FILESYSTEM
#include "./FileSystem.h"
#endif

#include <fstream>
#include <iostream>
using namespace std;

int main() {
    fstream fs;
    fs.open("./a.txt", ios::binary | ios::out | ios::in);

    // while(!fs.eof()) {
    //     char p[2];
    //     fs.read(p, 1);
    //     cout << (int)p[0] << endl;
    // }
    cout << sizeof(DiskINode) << endl;

    fs.close();

    return 0;
}