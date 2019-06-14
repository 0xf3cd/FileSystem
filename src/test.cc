#include <iostream>
#include <string.h>
using namespace std;

int main() {
    char temp[28] = {'a', '!', 0};
    string p;
    p = temp;
    cout << p.length() << endl;
    cout << p << endl;
    cout << (p[p.length()] == '\0') << endl;

    return 0;
}