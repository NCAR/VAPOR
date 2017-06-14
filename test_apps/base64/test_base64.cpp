#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <vapor/Base64.h>

using namespace Wasp;

int main(int argc, char **argv) {
    const int sz = 1024;

    string str;
    int buf[sz];
    int outbuf[sz];
    Base64 base64;

    for (int i = 0; i < sz; i++) {
        buf[i] = i;
    }

    base64.Encode((unsigned char *)buf, sz * 4, str);

    cout << str;

    size_t n;
    if (base64.Decode(str, (unsigned char *)outbuf, &n) < 0) {
        cerr << "Base64::Decode() : " << Base64::GetErrMsg() << endl;
        exit(1);
    }

    for (int i = 0; i < sz; i++) {
        if (buf[i] != outbuf[i]) {
            cerr << "Buf mismatch at offset " << i << endl;
        }
    }
    exit(0);
}
