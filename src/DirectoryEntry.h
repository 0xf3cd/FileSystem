#ifndef DIRECTORYENTRY
#define DIRECTORYENTRY

struct DirectoryEntry {
public:
    char fname[28] = {0}; // 初始化为全0
    int ino;
};

#endif
