#include <string>
#include <iostream>
#include "unique_ptr_cache.hpp"

class Obj
{
public:
    int val;
    Obj( int v ) : val( v ) 
    {
        std::cout << "construct " << val << std::endl;
    }
    ~Obj()
    {
        std::cout << "destruct " << val << std::endl;
    }
};

int main()
{
    unique_ptr_cache<std::string, Obj> c1(3);
    c1.insert( "aaa", new Obj(1) );
    c1.insert( "bbb", new Obj(2) );
    c1.insert( "ccc", new Obj(3) );

    c1.print();

    auto* p = c1.find("aaa");
    c1.print();

    c1.insert( "ccc", new Obj(33) );
    c1.print();

    c1.insert( "ddd", new Obj(4) );
    c1.print();

    auto* p2 = c1.find("aaa");
    c1.print();
}
