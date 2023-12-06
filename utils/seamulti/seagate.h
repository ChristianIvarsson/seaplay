#ifndef __SEAGATE_H__
#define __SEAGATE_H__

#include "rpi.h"

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
    char stLev;
public:
    explicit seagate_st();

    bool actDiag();

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
