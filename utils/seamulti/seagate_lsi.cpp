#include "seagate.h"
#include "str.h"



// Table is 128 bytes

/*
 1 - (resistor in series) Hi pull hi. _NOT_ connected to 3.3v rail so it's logically driven
 2 - +5v
 3 - Ser TX (38400)
 4 - Ser RX (38400)
 5 - Ground
key - - - - - - - - front <- - - -> board
 6 - Medium pull towards ground                    ( Has optional pull to ground )
 7 - Medium pull towards ground
 8 - Weak drive towards 3.3v
 9 - _EN_DBG_      ( Active low )
10 - +12v

Boot Cmds:
 AP <addr>    - < set Access Pointer >
 WT <data>    - < Write (data) starting from Access Pointer >
 RD           - < Read one byte from Access Pointer, post-increment pointer >
 GO           - < Run code at access pointer >
 TE           - < Toggle Echo >
 BR <divisor> - < Serial port baud >
 BT           - < Boot external eeprom >
 ?            - < Show this list >
 RET          - < Repeat last command > */


seagate_lsi::seagate_lsi() {
}

/*
> AP 1
Addr Ptr = 0x00000001     ( echo on )

> RD
0x18                      ( echo off )
Addr 0x00000000 = 0x18    ( echo on )

> WT 18
>                         ( echo off )
Addr 0x00000000 = 0x18    ( echo on )
*/


/*
Command: TE
Expected response: "Echo off" or "Echo on"
*/

bool seagate_lsi::setEcho( bool state ) {
    char buf[256];
    u32 tries = 4;

sendAgain:
    if ( sendLine( "TE\r", buf, sizeof(buf) ) ) {

        asciiToLower( buf );
        asciiStrip( buf );

        if ( strcmp( buf, "echooff" ) == 0 ) {
            echoState = false;
            // Got what we wanted!
            if ( state == false )
                return true;
            if ( tries-- )
                goto sendAgain;
        }
        else if ( strcmp( buf, "echoon" ) == 0 ) {
            echoState = true;
            // Got what we wanted!
            if ( state == true )
                return true;
            if ( tries-- )
                goto sendAgain;
        }

        printf("setEcho received unknown: '%s'\n", buf);
    }

    printf("setEcho could not send line\n");
    return false;
}

// addrptr = 0xf0000000
bool seagate_lsi::setAddress( u32 address ) {
    char buf[256], snd[32];
    u32 cmpAddr, tries = 4;
    s32 stVal;
    size_t stLen;

    if ( sprintf( snd, "AP %08X\r", address ) <= 0 ) {
        printf("setAddress sprintf failure\n");
        return false;
    }

sendAgain:
    if ( sendLine( snd, buf, sizeof(buf) ) ) {
        asciiToLower( buf );
        asciiStrip( buf );

        if ( (stVal = strncmp( buf, "addrptr=", 8 )) != 0 ) {
            if ( tries-- )
                goto sendAgain;
            printf("setAddress unexpected response (%d)\n", stVal);
            return false;
        }

        asciiClearLeading( buf, 8 );
        cmpAddr = fromAsciiHex( buf );

        if ( cmpAddr != address ) {
            if ( tries-- )
                goto sendAgain;
            printf("setAddress echoed address not the same as the requested one\n");
            return false;
        }

        // printf("setAddress: '%s' (%x)\n", buf, cmpAddr);
        return true;
    }

    return false;
}



/*
> RD
0x18
> TE
Echo on
> RD
Addr 0x00000001 = 0xF0
>
*/

bool seagate_lsi::readMem( u8 *data, u32 address, size_t len, bool sendAddress ) {

    u32 currAddr, datVal, tries = 4;
    size_t currLen, stripLen;
    char buf[256], tmp[32], *chPtr;
    bool res;
    s32 stVal;


    if ( sendAddress ) {
sendAgain:
        if ( !setAddress( address ) ) {
            if ( tries-- )
                goto sendAgain;
            printf("readMem failed to send address\n");
            return false;
        }
    }

    currAddr = address;
    currLen = 0;

    while (currLen < len) {

        // It'll repeat the last typed command if you only send CR
        if (currLen == 0) {
            res = sendLine( "RD\r", buf, sizeof(buf) );
        } else {
            res = sendLine( "\r", buf, sizeof(buf) );
        }

        if ( !res ) {
            if ( tries-- ) {
                printf("..\n");
                goto sendAgain;
            }
            printf("readMem failed command\n");
            return false;
        }

        asciiToLower( buf );

        if ( echoState ) {

            // > Addr < 0x00000001 = 0xF0
            if ( (stVal = strncmp( buf, "addr", 4 )) != 0 ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("readMem unexpected response (%d)\n", stVal);
                return false;
            }

            asciiClearLeading( buf, 4 );


            // Addr > 0x00000001 < = 0xF0
            stripLen = asciiExtract( buf, tmp );

            if ( !isAsciiHex(tmp) || fromAsciiHex( tmp ) != currAddr ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("readMem unexpected address response\n");
                return false;
            }

            asciiClearLeading( buf, stripLen );


            // Addr 0x00000001 > = < 0xF0
            stripLen = asciiExtract( buf, tmp );

            if ( (stVal = strncmp( tmp, "=", 1 )) != 0 ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("readMem unexpected '=' response (%d)\n", stVal);
                return false;
            }

            asciiClearLeading( buf, stripLen );
        }


        // Addr 0x00000001 = > 0xF0 <
        stripLen = asciiExtract( buf, tmp );

        if ( !isAsciiHex( tmp ) || (datVal = fromAsciiHex( tmp )) > 0xff ) {
            if ( tries-- ) {
                printf("..\n");
                goto sendAgain;
            }
            printf("readMem unexpected data response\n");
            return false;
        }

        data[ currLen ] = (u8)datVal;

        currLen++;
        currAddr++;
    }

    return true;
}

/*
> WT 0
Addr 0x00000000 = 0x00
> TE
Echo off
> WT 0
>     <--- This tripped up sendLine big time!
*/
bool seagate_lsi::writeMem( u8 *data, u32 address, size_t len, bool sendAddress ) {
    u32 currAddr, datVal, tries = 4;
    size_t currLen, stripLen;
    char buf[256], tmp[32];
    s32 stVal;

    if ( sendAddress ) {
sendAgain:
        if ( !setAddress( address ) ) {
            if ( tries-- )
                goto sendAgain;
            printf("writeMem failed to send address\n");
            return false;
        }
    }

    currAddr = address;
    currLen = 0;

    while (currLen < len) {

        if ( sprintf( buf, "WT %02X\r", data[ currLen ] ) <= 0 ) {
            printf("writeMem sprintf failure\n");
            return false;
        }

        if ( !sendLine( buf, buf, sizeof(buf), !echoState ) ) {
            if ( tries-- ) {
                printf("..\n");
                goto sendAgain;
            }
            printf("writeMem failed command\n");
            return false;
        }

        asciiToLower( buf );

        if ( echoState ) {

            // > Addr < 0x00000001 = 0xF0
            if ( (stVal = strncmp( buf, "addr", 4 )) != 0 ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("writeMem unexpected response (%d)\n", stVal);
                return false;
            }

            asciiClearLeading( buf, 4 );


            // Addr > 0x00000001 < = 0xF0
            stripLen = asciiExtract( buf, tmp );

            if ( !isAsciiHex(tmp) || fromAsciiHex( tmp ) != currAddr ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("writeMem unexpected address response\n");
                return false;
            }

            asciiClearLeading( buf, stripLen );


            // Addr 0x00000001 > = < 0xF0
            stripLen = asciiExtract( buf, tmp );

            if ( (stVal = strncmp( tmp, "=", 1 )) != 0 ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("writeMem unexpected '=' response (%d)\n", stVal);
                return false;
            }

            asciiClearLeading( buf, stripLen );


            // Addr 0x00000001 = > 0xF0 <
            stripLen = asciiExtract( buf, tmp );

            if ( !isAsciiHex( tmp ) || (datVal = fromAsciiHex( tmp )) > 0xff || datVal != data[ currLen ] ) {
                if ( tries-- ) {
                    printf("..\n");
                    goto sendAgain;
                }
                printf("writeMem unexpected data response\n");
                return false;
            }
        }

        currLen++;
        currAddr++;
    }

    return true;
}

bool seagate_lsi::setBaud( u32 baud ) {

    char tmp[32];

    if ( sprintf( tmp, "BR %02X\r", baud ) <= 0 ) {
        printf("setBaud sprintf failure\n");
        return false;
    }

    if ( !oneShot( tmp ) ) {
        printf("setBaud failed command\n");
        return false;
    }

    return true;
}

