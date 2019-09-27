#include <string>
#include "unique_ptr_cache.hpp"

class Obj
{
public:
    int val;
    Obj( int v ) : val( v ) {}
};

int main()
{
    unique_ptr_cache<std::string, Obj> c1(5);
    c1.insert( "haha", new Obj(4) );

}
