#ifndef ENCODING_H
#define ENCODING_H
#include "windows.h"
#include <QString>

class Encoding
{
public:
    Encoding();
    static QString ANSIToUTF8(const char* str);
private:
    static wchar_t * ANSIToUnicode( const char* str );
    static char * UnicodeToUTF8( const wchar_t* str );
};

#endif // ENCODING_H
