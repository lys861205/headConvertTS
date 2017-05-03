#include "encoding.h"

Encoding::Encoding()
{
}

//зЊТы
wchar_t * Encoding::ANSIToUnicode( const char* str )
{
    int textlen ;
    wchar_t * result;
    textlen = MultiByteToWideChar( CP_ACP, 0, str,-1, NULL,0 );
    result = (wchar_t *)malloc((textlen+1)*sizeof(wchar_t));
    memset(result,0,(textlen+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0,str,-1,(LPWSTR)result,textlen );
    return result;
}

char * Encoding::UnicodeToUTF8( const wchar_t* str )
{
    char* result;
    int textlen;
    textlen = WideCharToMultiByte( CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL );
    result =(char *)malloc((textlen+1)*sizeof(char));
    memset(result, 0, sizeof(char) * ( textlen + 1 ) );
    WideCharToMultiByte( CP_UTF8, 0, str, -1, result, textlen, NULL, NULL );
    return result;
}
QString Encoding::ANSIToUTF8(const char* str)
{
    char *str_utf8 = NULL;
    wchar_t * str_unicode = NULL;

    str_unicode = ANSIToUnicode(str);
    if (NULL != str_unicode)
    {
        str_utf8 = UnicodeToUTF8(str_unicode);
        free(str_unicode);
        str_unicode = NULL;
    }
    QString strUtf8(str_utf8);
    if (str_utf8)free(str_utf8);
    return strUtf8;
}
