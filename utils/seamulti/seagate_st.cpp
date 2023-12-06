
#include "seagate.h"
#include "str.h"

seagate_st::seagate_st() {
    printf("seagate_st()\n");
    stLev = '?';
}

// Send CTRL-Z and expect "T>" in response
bool seagate_st::actDiag() {
    char buf[128];

    // Send ctrl - z to enable diagnostics
    if ( sendLineManual( "\x1a\r", buf, sizeof(buf) ) ) {
        asciiToLower( buf );
        asciiStrip( buf );
        if (strncmp("t>", buf, 2) != 0) {
            printf("Could not activate diagnostics\n");
            return false;
        }

        stLev = 't';
        return true;
    }

    return false;
}

/*
+ffff,0000
BE
T>
*/
bool seagate_st::readMem( u8 *data, u32 address, size_t len, bool sendAddress ) {
    size_t stLen;
    u32 tries = 4;
    char buf[256], snd[32];

    if ( len == 0 ) {
        printf("Can't request read of 0 bytes!\n");
        return false;
    }

    while ( len > 0 ) {
        if ( sprintf( snd, "+%04x,%04x\r", (u32)(address>>16), (u32)(address&0xffff) ) <= 0 ) {
            printf("readMem sprintf failure\n");
            return false;
        }

        if ( !sendLineManual( snd, buf, sizeof(buf) ) ) {
            if ( tries-- )
                continue;
            printf("Could not read\n\r");
            return false;
        }

        asciiToLower( buf );
        asciiStrip( buf );

        if ( buf[0] != stLev || buf[1] != '>' ) {
            printf("readMem unexpected return string: %s\n", buf);
            if ( tries-- )
                continue;
            printf("Gave up!\n");
            return false;
        }

        asciiClearLeading( buf, 2 );
        stLen = strlen( snd );

        if ( strncmp( snd, buf, 10) != 0) {
            printf("strncmp wrr\n");
            return false;
        }

        // Fetch second line
        snd[0] = 0;
        if ( !sendLineManual( snd, buf, sizeof(buf) ) ) {
            if ( tries-- )
                continue;
            printf("Could not read\n\r");
            return false;
        }

        asciiToLower( buf );
        asciiStrip( buf );
        stLen = strlen( buf );

        if (stLen != 2 || !isAsciiHex( buf )) {
            printf("Unexpected data response: %s (%lu)\n", buf, stLen);
            if ( tries-- )
                continue;
            printf("Could not read\n\r");
            return false;
        }

        *data++ = (u8)fromAsciiHex( buf );

        address++;
        len--;
    }

    return true;
}


bool seagate_st::writeMem( u8 *data, u32 address, size_t len, bool sendAddress ) {
    return false;
}
