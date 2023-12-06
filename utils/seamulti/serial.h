#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "rpi.h"

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

class serial {
private:
    int sPort;

public:
    explicit serial();
    ~serial();

    bool open( const char *devStr, speed_t speed );
    bool close();

    bool flush();

    bool sendLine( const char *snd, char *rec, size_t recSize, bool noResp = false );
    bool sendLineManual( const char *snd, char *rec, size_t recSize );
    bool oneShot( const char *snd );


};



#endif
