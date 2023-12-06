#ifndef __STR_H__
#define __STR_H__

#include "rpi.h"

static bool isAsciiHex(const char *str) {
    size_t stLen;
    if ( str == 0 )
        return false;
    while ( *str == ' ' || *str == 0x09 )
        str++;
    if ( *str == 0  )
        return false;

    stLen = strlen( str );
    if (stLen > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        str+=2;

    while ( *str != 0 ) {
        if ( (*str >= '0' && *str <= '9') ||
             (*str >= 'A' && *str <= 'F') ||
             (*str >= 'a' && *str <= 'f')  ) {
            str++;
            continue;
        }; 
        return false;
    }
    return true;
}

static u32 fromAsciiHex( const char *str ) {
    size_t stLen;
    u32 retVal = 0;
    if ( str == 0 )
        return 0;
    while ( *str == ' ' || *str == 0x09 )
        str++;
    if ( *str == 0  )
        return 0;

    stLen = strlen( str );
    if (stLen > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        str+=2;

    while ( *str != 0 ) {
        retVal <<= 4;
        if      ( *str >= '0' && *str <= '9' )   retVal |= (*str - '0');
        else if ( *str >= 'A' && *str <= 'F' )   retVal |= (*str - 0x37);
        else if ( *str >= 'a' && *str <= 'f' )   retVal |= (*str - 0x57);
        else return 0;
        str++;
    }
    return retVal;
}


static void asciiToLower( char *str ) {
    size_t stLen = strlen( str );
    for ( size_t i = 0; i < stLen; i++ ) {
        if ( str[ i ] >= 'A' && str[ i ] <= 'Z' )
            str[ i ] += 0x20;
    }
}

static void asciiStrip( char *str ) {
    size_t stLen = strlen( str );
    size_t i = 0;

    // Terminating 0 is not part of str len.
    // Expect to always move that
    while ( i < stLen ) {
        if ( str[ i ] < 0x21 || str[ i ] > 0x7e ) {
            // Include terminating 0
            for (size_t u = i; u < stLen; u++)
                str[ u ] =  str[ u + 1 ];
            // Whelp. No more!
            if ( str[ i ] == 0 )
                break;
            stLen--;
        } else {
            i++;
        }
    }
}

static void asciiClearLeading( char *str , size_t toClear ) {
    size_t stLen = strlen( str );

    if ( stLen <= toClear ) {
        *str = 0;
        return;
    }

    stLen -= toClear;

    for (size_t i = 0; i <= stLen; i++) {
        str[i] = str[i + toClear];
    }
}


static char *asciiFirstChar( char *inString ) {
    // size_t skipped = 0;

    while ( *inString != 0 && (*inString <= ' ' || *inString > 0x7e) )
        inString++;

    return inString;
}

/*
Important!
This thing will count the total numbers of read characters, including skipped ones!
This is to enable easy use of asciiClearLeading()
*/

static size_t asciiExtract( char *src, char *dst ) {
    size_t strLen = 0;

    while ( *src != 0 && (*src <= ' ' || *src > 0x7e) ) {
        src++;
        strLen++;
    }

    while ( *src > ' ' && *src < 0x7f ) {
        *dst++ = *src++;
        strLen++;
    }

    *dst = 0;

    return strLen;
}




#endif
