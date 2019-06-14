#include "UserInterface.h"
#include <iostream>
#include <unistd.h>

vector<string> UserInterface::splitString(const string &s, const string &seperator) { 
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

UserInterface::UserInterface(FileOperator* pFO) {
    FO = pFO;
}

UserInterface::~UserInterface() {

}

/**
 * 处理用户输入
 */
void UserInterface::processInput(string input) {
    if(input.length() == 0) {
        cout << endl;
    }
    auto sv = splitString(input, " ");
    const int para_amount = sv.size() - 1; // 参数个数
    const string cmd = sv[0];

    // cout << sv.size() << endl;
    for(int i = 0; i < sv.size(); i++) {
        const string s = sv[i];
        auto pos = s.find("/");
        if(pos != string::npos) {
            cout << "目前不支持多层路径，如 cd ./folder/f1/" << endl;
            cout << "请使用其他指令代替，如 cd folder, cd f1" << endl;
            return;
        }
    }

    if(cmd == "format") {
        if(para_amount == 0) {
            FO -> format();
            cout << "格式化成功" << endl;
        } else {
            cout << "format 不能接受参数" << endl;
        } 
        return;
    }

    if(cmd == "pwd") {
        if(para_amount == 0) {
            cout << FO -> pwd() << endl;
        } else {
            cout << "pwd 不能接受参数" << endl;
        } 
        return;
    }


    if(cmd == "ls") {
        if(para_amount == 0) {
            auto lsr = FO -> ls();
            for(auto it = lsr.begin(); it != lsr.end(); it++) {
                cout << *it << endl;
            }
        } else {
            cout << "ls 不能接受参数" << endl;
        } 
        return;
    }

    if(cmd == "mkdir") {
        if(para_amount == 1) {
            int res = FO -> mkdir(sv[1]);
            if(res == -1) {
                cout << "存在同名文件！" << endl;
            }
        } else {
            cout << "mkdir 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "fcreate") {
        if(para_amount == 1) {
            int res = FO -> fcreate(sv[1]);
            if(res == -1) {
                cout << "存在同名文件！" << endl;
            }
        } else {
            cout << "fcreate 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "fopen") {
        if(para_amount == 1) {
            int res = FO -> fopen(sv[1]);
            if(res == -1) {
                cout << "不存在这个文件" << endl;
            } else if(res == -2) {
                cout << "这是个文件夹" << endl;
            }
        } else {
            cout << "fopen 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "fclose") {
        if(para_amount == 0) {
            FO -> fclose();
        } else {
            cout << "fclose 不能接受参数" << endl;
        } 
        return;
    }

    if(cmd == "fread") {
        if(para_amount == 1) {
            int p1 = std::stoi(sv[1]);
            char* buf = new char[p1];
            int res = FO -> fread(buf, p1);
            cout << "读出 " << res << " 字节" << endl;
            for(int i = 0; i < res; i++) {
                cout << buf[i];
            }
            cout << endl;
            delete[] buf;
        } else {
            cout << "fread 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "fwrite") {
        if(para_amount == 0) {
            // string to_write;
            // cin >> to_write;
            string to_write = "";
            char x;
            while((x = getchar()) != 10) {
                to_write += x;
            }
            
            int length = to_write.length();
            char* buf = new char[length];
            int i;
            for(i = 0; i < length; i++) {
                buf[i] = to_write[i];
            }
            int res = FO -> fwrite(buf, length);
            cout << "写入 " << res << " 字节" << endl;
            delete[] buf;
        } else {
            cout << "fwrite 不接受参数" << endl;
        }
        
        return;
    }

    if(cmd == "flseek") {
        if(para_amount == 1) {
            int p1 = std::stoi(sv[1]);
            FO -> flseek(p1);
        } else {
            cout << "flseek 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "curseek") {
        if(para_amount == 0) {
            cout << FO -> curseek() << endl;
        } else {
            cout << "curseek 不接受参数" << endl;
        }
        return;
    }

    if(cmd == "fsize") {
        if(para_amount == 0) {
            int res = FO -> fsize();
            if(res == -1) {
                cout << "没有打开任何文件" << endl;
                return;
            }
            cout << "文件大小为 " << res << " 字节" << endl;
        } else {
            cout << "fsize 不接受参数" << endl;
        }
        return;
    }

    if(cmd == "fdelete") {
        if(para_amount == 1) {
            int res = FO -> fdelete(sv[1]);
            if(res == -1) {
                cout << "要删除文件不存在" << endl;
            } else if(res == -2) {
                cout << "要删除的是文件夹，请使用 dirdelete 删除" << endl;
            }
        } else {
            cout << "fdelete 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "dirdelete") {
        if(para_amount == 1) {
            int res = FO -> fdelete(sv[1]);
            if(res == -1) {
                cout << "要删除文件夹不存在" << endl;
            } else if(res == -2) {
                cout << "要删除的是普通文件，请使用 fdelete 删除" << endl;
            }
        } else {
            cout << "dirdelete 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "cd") {
        if(para_amount == 0) {
            FO -> goroot();
        } else if(para_amount == 1) {
            int res = FO -> cd(sv[1]);
            if(res == -1) {
                cout << "文件夹不存在" << endl;
            } else if(res == -2) {
                cout << "参数应当为文件夹名" << endl;
            }
        } else {
            cout << "fdelete 接受 1 个参数" << endl;
        }
        return;
    }

    if(cmd == "cp") {
        if(para_amount == 2) {
            int res = FO -> cp(sv[1], sv[2]);
            if(res == -1) {
                cout << "要复制的文件/文件夹不存在" << endl;
            } else if(res == -2) {
                cout << "目的文件夹不存在" << endl;
            }
        } else {
            cout << "cp 接受 2 个参数" << endl;
        }
        return;
    }

    if(cmd == "mv") {
        if(para_amount == 2) {
            int res = FO -> cp(sv[1], sv[2]);
            if(res == -1) {
                cout << "要移动的文件/文件夹不存在" << endl;
            } else if(res == -2) {
                cout << "目的文件夹不存在" << endl;
            }
        } else {
            cout << "mv 接受 2 个参数" << endl;
        }
        return;
    }

    if(cmd == "goroot") {
        if(para_amount == 0) {
            FO -> goroot();
        } else {
            cout << "goroot 不接受参数" << endl;
        }
        return;
    }

    cout << "不支持的指令：" << cmd << endl;
}

void UserInterface::start() {
    while(true) {
        string cur_dir = FO -> pwd();
        if(FO -> isOpeningFile()) {
            cout << "[" << cur_dir << "] $file opened$ # ";             
        } else {
            cout << "[" << cur_dir << "] # ";
        }

        string input = "";
        char x;
        while((x = getchar()) != 10) {
            input += x;
        }

        if(input.length() == 0) {
            continue;
        }

        processInput(input);
    }
}
