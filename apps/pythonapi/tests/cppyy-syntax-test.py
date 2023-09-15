import cppyy
from ctypes import c_int


cppyy.cppdef("""
#include <string>
using std::string;

void p(string s) {
    printf("CPP %s\\n", s.c_str());
}

void testVoid(void (*cb)(void)) {
    printf("TEST VOID\\n");
    cb();
}

void testRet(string (*cb)(void)) {
    string s = cb();
    printf("TEST RET = %s\\n", s.c_str());
}

void testIn(string (*cb)(string)) {
    string s = cb("CPP STR");
    printf("TEST RET = %s\\n", s.c_str());
}

void testInConstRef(string (*cb)(const string &)) {
    string s = cb("CPP STR");
    printf("CONST TEST RET = %s\\n", s.c_str());
}

""")

# cppyy.gbl.p("hello")

def cb():
    print("Python Callback")
    return "<Python Return Value>"

def cb2(s):
    print(f"Python Callback({s})")
    # s = "d"
    return f"<Python Return Value={s}>"
    # raise FileNotFoundError
    return None

cppyy.gbl.testVoid(cb)
cppyy.gbl.testRet(cb)
cppyy.gbl.testIn(cb2)
cppyy.gbl.testInConstRef(cb2)

exit()


cppyy.cppdef("""
void ref_int(int &a) {
    a = 42;
}

#include <string>
using std::string;

void ref_str(string &s) {
    s = "hello";
}
""")

a = c_int(3)
cppyy.gbl.ref_int(a)
print(a)

# does not work
# s = "bye"
# cppyy.gbl.ref_str(s)
# print(s)

s = cppyy.gbl.std.string("bye")
cppyy.gbl.ref_str(s)
print(s)
