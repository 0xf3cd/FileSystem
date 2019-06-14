#ifndef USERINTERFACE
#define USERINTERFACE

#include "FileOperator.h"
#include <string>
#include <vector>

class UserInterface {
private:
    FileOperator* FO;

private:
    vector<string> splitString(const string &s, const string &seperator);

public: 
    UserInterface(FileOperator* pFO);
    ~UserInterface();

    /**
     * 处理用户输入
     */
    void processInput(string input);

    /**
     * 开始工作
     */
    void start();
};

#endif