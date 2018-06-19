#ifndef STDTOSTRING_H
#define STDTOSTRING_H
#include <string>
#include <sstream>

using namespace std;
namespace std
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }

}



#endif

