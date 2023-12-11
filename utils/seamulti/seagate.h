#ifndef __SEAGATE_H__
#define __SEAGATE_H__

#include "shared.h"

#include "serial.h"


class seagate_base : public serial {
public:
    virtual bool readMem( u8 *data, u32 address, size_t len, bool sendAddress = true ) = 0;
    virtual bool writeMem( u8 *data, u32 address, size_t len, bool sendAddress = true ) = 0;
};

// ST specific
class seagate_st
    : public seagate_base {
private:
    char m_diagLev;
    size_t m_blockSz;


    bool readBytes( u8 *data, u32 address, size_t len );
    bool readBlock( u8 *data, u32 address, size_t len );

public:
    explicit seagate_st();

    bool actDiag();
    bool setDiagLevel( char lev );

    // This is bound to m_blockSz and made read-only to prevent external code from changing it
    const size_t & blkDumpSz;

// Present in all
    bool readMem( u8 *data, u32 address, size_t len, bool sendAddress = true );
    bool writeMem( u8 *data, u32 address, size_t len, bool sendAddress = true );
};

// LSI specific
class seagate_lsi
    : public seagate_base {
private:
    bool echoState;

public:
    explicit seagate_lsi();

    bool setEcho( bool state );
    bool setAddress( u32 address );

    bool setBaud( u32 baud );

// Present in all
    bool readMem( u8 *data, u32 address, size_t len, bool sendAddress = true );
    bool writeMem( u8 *data, u32 address, size_t len, bool sendAddress = true );
};

#endif
