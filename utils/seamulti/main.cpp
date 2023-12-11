#include "shared.h"

#include "seagate.h"
// #include "serial.h"

#include "str.h"

/*

seag_test  < /dev/ser_dev >  < target lsi/st >  < op r / w >  < address >  < length >  < file >

Caveat: Setting length to 0 while writing will use the file length

*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shared

bool dumpTarget(seagate_base & seg, FILE *fp, u32 address, size_t len) {

    size_t readChunk, readLen = 0;
    u8 buf[ 768 ];

    if ( len == 0 ) {
        printf("Can't dump 0 bytes\n");
        return false;
    }

    printf("Dumping\nAddress: %x\nLength: %lu\n\n\n", address, len);

    while (readLen < len) {
        readChunk = ((len - readLen) > 512) ? 512 : (len - readLen);

        printf("Address: %08x (%lu)..\n", address, len - readLen);
        if ( !seg.readMem( buf, address, readChunk, false ) ) {
            printf("Could not dump\n");
            return false;
        }

        if ( fwrite( buf, 1, readChunk, fp ) != readChunk ) {
            printf("Could not write to file\n");
            return false;
        }

        address += readChunk;
        readLen += readChunk;
    }

    printf("Done\n");

    return true;
}

bool uploadTarget(seagate_base & seg, FILE *fp, size_t fileSize, const u32 address, size_t len) {

    size_t writeChunk, writeLen = 0, writeAddr = address;
    u8 buf[ 768 ];

    if ( len == 0 )
        len = fileSize;

    printf("Writing\nAddress: %x\nLength: %lu\n\n\n", address, len);

    while (writeLen < len) {
        writeChunk = ((len - writeLen) > 512) ? 512 : (len - writeLen);

        if ( fread( buf, 1, writeChunk, fp ) != writeChunk ) {
            printf("Could not read from file\n");
            return false;
        }

        printf("Address: %08x (%lu)..\n", writeAddr, len - writeLen);
        if ( !seg.writeMem( buf, writeAddr, writeChunk, false ) ) {
            printf("Could not upload data\n");
            return false;
        }

        writeAddr += writeChunk;
        writeLen += writeChunk;
    }

    printf("Done\n");

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LSI specific

bool dump_lsi(seagate_lsi & seg, const char *file, u32 address, size_t len) {
    FILE *fp;
    bool retVal;

    if ( !seg.setEcho( false ) ) {
        printf("Could not configure echo for dumping\n");
        return false;
    }

    if ( !seg.setAddress( address ) ) {
        printf("Could not configure address for dumping\n");
        return false;
    }

    if ( (fp = fopen(file, "wb")) == nullptr ) {
        printf("Could not open file for writing\n");
        return false;
    }

    rewind(fp);

    retVal = dumpTarget(seg, fp, address, len);

    fclose(fp);

    return retVal;
}

bool upload_lsi(seagate_lsi & seg, const char *file, const u32 address, size_t len) {

    FILE *fp;
    size_t fileSize;
    bool retval;

    if ( !seg.setEcho( false ) ) {
        printf("Could not configure echo for uploading\n");
        return false;
    }

    if ( !seg.setAddress( address ) ) {
        printf("Could not configure address for uploading\n");
        return false;
    }

    if ( (fp = fopen(file, "rb")) == nullptr ) {
        printf("Could not open file for reading\n");
        return false;
    }

    fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
    rewind(fp);

    if ( fileSize == 0 || fileSize < len ) {
        printf("File size or specified length out of range!\n");
        fclose(fp);
        return false;
    }

    if ( (retval = uploadTarget(seg, fp, fileSize, address, len)) ) {
        if ( !(retval = seg.setAddress( address )) )
            printf("Could not configure address for jump\n");
    }

    fclose(fp);
    return retval;
}

int sea_lsi(char *argv[]) {

    bool retVal = false;

    seagate_lsi seg;

    // seag_test    --< /dev/ser_dev >--       < lsi >  < op r / w >  < address >  < length >  < file >
    seg.open( argv[1], B38400 );

    seg.flush();

    // 162 - 38400

    // 115200
    // seg.setBaud( 54 );
    // seg.open( argv[1], B115200 );

    // 230400
    seg.setBaud( 27 );
    seg.open( argv[1], B230400 );

    // B230400
    usleep(125 * 1000);

    printf("setEcho\n");
    seg.setEcho( true );
    seg.setEcho( false );

    // seag_test  < /dev/ser_dev >  < lsi >  < op r / w >  < address >  < length >  < file >
    if (*argv[3] == 'r' || *argv[3] == 'R') {
        retVal = dump_lsi( seg, argv[6], fromAsciiHex( argv[4] ), fromAsciiHex( argv[5] ) );
    }
    else if (*argv[3] == 'w' || *argv[3] == 'W') {
        retVal = upload_lsi( seg, argv[6], fromAsciiHex( argv[4] ), fromAsciiHex( argv[5] ) );
    }

    // printf("-- W A I T I N G --\n");
    // sleep( 1 );

    seg.close();

    return retVal ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST specific

bool dump_st(seagate_st & seg, const char *file, u32 address, size_t len) {
    FILE *fp;
    bool retVal;

    if ( (fp = fopen(file, "wb")) == nullptr ) {
        printf("Could not open file for writing\n");
        return false;
    }

    rewind(fp);

    retVal = dumpTarget(seg, fp, address, len);

    fclose(fp);

    return retVal;
}

int sea_st(char *argv[]) {

    bool retVal = false;

    seagate_st seg;

    // seag_test    --< /dev/ser_dev >--       < st >  < op r / w >  < address >  < length >  < file >
    if (!seg.open( argv[1], B9600 ))
        goto closeDevice;

    seg.flush();

    printf("Testing\n");
    
    if (!seg.actDiag())
        goto closeDevice;

    // seag_test  < /dev/ser_dev >  < st >  < op r / w >  < address >  < length >  < file >
    if (*argv[3] == 'r' || *argv[3] == 'R') {
        seg.setDiagLevel( '1' );
        retVal = dump_st( seg, argv[6], fromAsciiHex( argv[4] ), fromAsciiHex( argv[5] ) );
    }
    else if (*argv[3] == 'w' || *argv[3] == 'W') {
        // retVal = upload_lsi( seg, argv[6], fromAsciiHex( argv[4] ), fromAsciiHex( argv[5] ) );
    }

closeDevice:
    seg.close();

    return retVal ? 0 : 1;
}

int main(int argc, char *argv[]) {

    if (    argc != 7 ||                                                                  // Always 7 arguments
            (*argv[3] != 'r' && *argv[3] != 'R' && *argv[3] != 'w' && *argv[3] != 'W') || // Only r / w are supported
            !isAsciiHex( argv[4] ) || !isAsciiHex( argv[5] ) ||                           // Address and length must both qualify as numbers
            ((*argv[3] == 'r' || *argv[3] == 'R') && fromAsciiHex( argv[5] ) == 0) )      // Read has a minimum length of 1
    {
        printf("\nUsage:\n");
        printf("seag_test  < /dev/ser_dev >  < target lsi / st >  < op r / w >  < address H >  < length H >  < file >\nSetting length to 0 while writing will determine length from file size\n\n\n\n");
        return 1;
    }

    if ( strncmp("lsi", argv[2], 3) == 0 || strncmp("LSI", argv[2], 3) == 0 ) {
        return sea_lsi( argv );
    } else if ( strncmp("st", argv[2], 2) == 0 || strncmp("ST", argv[2], 2) == 0 ) {
        return sea_st( argv );
    }

    return 1;
}
