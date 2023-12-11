
#include "seagate.h"
#include "str.h"

seagate_st::seagate_st()
    : blkDumpSz(m_blockSz) {
    printf("seagate_st()\n");
    m_blockSz = 0x220;
    m_diagLev = '?';
}

// Send CTRL-Z and expect "T>" in response
bool seagate_st::actDiag() {
    char buf[128];

    // Send ctrl - z to enable diagnostics
    if ( sendLineMK2( "\x1a\r", buf, sizeof(buf) ) ) {

        asciiToUpper( buf );
        asciiStrip( buf );
        
        if ( strlen( buf ) != 2 || buf[1] != '>' ) {
            printf("Could not activate diagnostics (ret: %s)\n", buf);
            return false;
        }

        printf("Target is currently at diagnostics level %c\n", (m_diagLev = buf[0]));
        return true;
    }

    return false;
}

bool seagate_st::setDiagLevel( char lev ) {

    char buf[64];

    if ( sprintf( buf, "/%c\r", lev ) <= 0 ) {
        printf("setDiagLevel sprintf failure\n");
        return false;
    }

    // Lowercase to uppercase
    if ( lev >= 'a' && lev <= 'z' )
        lev -= 0x20;

    if ( sendLineMK2( buf, buf, sizeof(buf) ) ) {

        asciiToUpper( buf );
        asciiStrip( buf );

        if ( strlen( buf ) < 2 || buf[0] != lev || buf[1] != '>' ) {
            printf("Could not activate diagnostics level '%c' (ret: %s)\n", lev, buf);
            return false;
        }

        printf("Target changed to diagnostics level %c\n", (m_diagLev = buf[0]));

        // Make sure the prompt is seen by consecutive commands
        printf("<< Resetting target prompt >>\n");
        sendLineMK2( "\r" );

        return true;
    }

    return false;
}




/*
1>Dffff,0000
 Addr  0 1 2 3  4 5 6 7  8 9 A B  C D E F 10 1 2 3  4 5 6 7  8 9 A B  C D E F        <- Only detect if "some" line is send before the data. 
FFFF0000 BE88E601 13038E77 1E60F78E 109BE6F1 18F7F6F1 C4FBE6F1 00F6F6F1 C6FBE6F1
FFFF0020 D6F7F6F1 C8FBE6F1 18F7F6F1 CAFBF20B C8FBF20A CAFBE609 D6F7BF88 E00FF6FF
FFFF0040 D02FE6FF C400E6FE 2AC0DC3F 983E982E 985E984E DC0F986E DAC79E01 DAC73601
FFFF0060 DAC536B2 DAC55A73 9A780CA0 AE7D9A7D FEA06E79 9A79FE60 E7F26C00 43F2D9FB
FFFF0080 EA337200 BE880AB5 C000E6F4 C500E6F3 2C1EE6F6 C500E6F5 361EE6F7 0404DAC0
FFFF00A0 C800E6F1 C081F6F1 04ECE6F1 124EF6F1 06ECBF88 9A7803A0 6E799A79 FE60BE88
FFFF00C0 0AC8C000 E6F6BAA5 F6F628C0 F6F616C0 7E774E77 DAC55C94 DAC5D8B6 DAC548C1
FFFF00E0 E6F4C500 E6F3341E E6F6C500 E6F5461E E6F70104 DAC0C800 E6F1C08A F6F100EC
FFFF0100 E6F1BC2F F6F102EC 9A7802A0 0AB5C040 BF88DAC5 7899E6FF C400E6FE 34C0DC3F
FFFF0120 983E982E 985E984E DC0F986E DAC79E01 DAC6B8C4 0D40E6F6 C40066F6 3BFF2D05
FFFF0140 E6F18000 1B61F2F6 0EFEE6F1 0002E6F7 000EF6F7 0EFE5B11 02F60EFE F2F50CFE
FFFF0160 F2F2502F 2D05E6F1 00021B12 02F60EFE E6F30002 1B36F2F3 0CFEF2F1 0EFEE6F6
FFFF0180 4A1EE6F7 C500E6F8 0800DC07 A826DAC4 E8140862 18700852 A0183DF7 DB00CC06
FFFF01A0 DC04A875 DC02B873 08321820 08521840 A0163DF5 DB00BE88 F68E9ADE E6F10060
FFFF01C0 F6F190DE E60806F7 F608A0FB E6F7C600 E6F60E96 E6F80008 E6F0D4DA F6F0A2FB
FFFF01E0 E6F0A2FB E609D6F7 F20BC8FB F20ACAFB E60880FB E6880068 E6F0D4DC FAC59273
FFFF0200 FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF
1>
*/



/*
+ffff,0000
BE
T>
*/
bool seagate_st::readMem( u8 *data, u32 address, size_t len, bool sendAddress ) {
    size_t stLen;
    char snd[32];

    if ( len == 0 ) {
        printf("Can't request read of 0 bytes!\n");
        return false;
    }

    while ( len > 0 ) {
        if ( sprintf( snd, "+%04X,%04X\r", (u32)(address>>16), (u32)(address&0xffff) ) <= 0 ) {
            printf("readMem sprintf failure\n");
            return false;
        }

        if ( !sendLineMK2( snd, snd, sizeof(snd) ) ) {
            printf("readMem: Could not request read\n");
            return false;
        }

        if ( strlen( snd ) > 2 ) {
            printf("readMem: Unexpected response length\n");
            return false;
        }

        if ( !isAsciiHex( snd ) ) {
            printf("Unexpected data response: %s (%lu)\n", snd, stLen);
            return false;
        }

        *data++ = (u8)fromAsciiHex( snd );

        address++;
        len--;
    }

    return true;
}

bool seagate_st::writeMem( u8 *data, u32 address, size_t len, bool sendAddress ) {
    return false;
}
