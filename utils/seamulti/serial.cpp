#include "serial.h"

#include <chrono>
using namespace std::chrono;

serial::serial() {
    // You can do this in the class since c++11
    sPort = -1;
}

// First serial port example I could find...
bool serial::open(const char *devStr, speed_t speed) {

    this->close();

    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    printf("Attempting acquisition of device: %s\n", devStr);

    if ((sPort = ::open(devStr, O_RDWR)) < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        this->close();
        return false;
    }

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() overwise behaviour
    // is undefined
    if (tcgetattr(sPort, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        this->close();
        return false;
    }

    tty.c_cflag &= ~( PARENB | CSTOPB | CSIZE ) ;
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

    tty.c_iflag &= ~(
        IXON | IXOFF |                                                    // Turn off s/w flow ctrl
        IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL ); // Disable any special handling of received bytes

    // tty.c_iflag |= (IXON | IXOFF); // Made things shit the bed

    tty.c_oflag &= ~(OPOST | ONLCR);
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)

    tty.c_cc[VTIME] = 1; // Wait for up to 100 ms
    tty.c_cc[VMIN] = 0;

    // Set baud rate
    cfsetspeed(&tty, speed);

    // Store settings
    if (tcsetattr(sPort, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        this->close();
        return false;
    }

    // Delay for 250 ms and then flush whatever is in buffer
    usleep(250 * 1000);
    tcflush(sPort, TCIOFLUSH);

    printf("Device open\n");

    return true;
}

// This weirdo came to be since it's not exactly known which state the target is in.
// If something happens in the middle of a message and we resume it's not going to interpret the '?' correctly
bool serial::flush() {
    // No need to send CR for this command for some reason
    static const unsigned char msg[] = "-\r?";
    s32 ioStat;
    u8  junk;

    // u32 rCnt = 0;
    // u8  recBuf[ 513 ];

    printf("-- F L U S H I N G --\n");

    if (sPort < 0)
        return false;

    for (u32 i = 0; i < 4; i++) {
        if ((ioStat = write(sPort, msg, 1)) != 1) {
            printf("Could not send '-'\n");
            return false;
        }
        usleep(250 * 1000);
    }

    if ((ioStat = write(sPort, &msg[1], 1)) != 1) {
        printf("Could not send '\\r'\n");
        return false;
    }

    usleep(250 * 1000);

    if ((ioStat = write(sPort, &msg[2], 1)) != 1) {
        printf("Could not send '?'\n");
        return false;
    }

    auto oldTime = system_clock::now();
    auto startTime = oldTime;
    auto newTime = oldTime;

    do {
        ioStat = read(sPort, &junk, 1);
        // Device error
        if ( ioStat < 0 ) {
            printf("Flush error\n");
            return false;
        }
        // Reset old time, still rec'ing
        else if ( ioStat > 0 ) {
            // if ( rCnt < (sizeof(recBuf) - 1) ) recBuf[ rCnt++ ] = junk;
            // printf("Rec'd: %x\n", junk);
            oldTime = system_clock::now();
        }

        newTime = system_clock::now();

        // This should only take around 50 - 250 ms. Give up after four seconds
        if ( duration_cast<milliseconds>(newTime - startTime).count() > 4000 ) {
            printf("Flush continuous flow of garbage characters!\n");
            return false;
        }

    } while ( duration_cast<milliseconds>(newTime - oldTime).count() < 500 );

    // recBuf[ rCnt ] = 0;
    // printf("%s\n", recBuf);
    // printf("-- D O N E --\n");

    return true;
}

// If re-implemented, make sure the user can use the same buffers for snd and rec
bool serial::sendLine( const char *snd, char *rec, size_t recSize, bool noResp ) {

    s32 ioStat;
    char ch;
    size_t rCnt = 0;
    size_t stLen = strlen( snd );

    // printf("Sending: '%s'\n", snd);
    if ( stLen > 0 && (ioStat = write(sPort, snd, stLen)) != stLen ) {
        printf("Could not send line!\n");
        return false;
    }

    auto oldTime = system_clock::now();
    auto startTime = oldTime;
    auto newTime = oldTime;

    if ( stLen > 0 ) {
        // Strip up to and including sent message
        do {
            ioStat = read( sPort, &ch, 1 );
            // printf("strip: %c\n", ch);
            // Device error
            if ( ioStat < 0 ) {
                printf("sendLine strip error\n");
                return false;
            }
            // Received something
            else if ( ioStat > 0 ) {
                if ( ch == snd[ rCnt ] ) {
                    if ( ++rCnt >= stLen )
                        break;
                } else {
                    rCnt = 0;
                }
            }

            if ( duration_cast<milliseconds>(system_clock::now() - startTime).count() > 250 ) {
                printf("sendLine strip took too long!\n");
                return false;
            }
        } while ( 1 );
    }

    rCnt = 0;

    do {
        ioStat = read(sPort, &ch, 1);
        // Device error
        if ( ioStat < 0 ) {
            printf("sendLine error\n");
            return false;
        }
        // Received something
        else if ( ioStat > 0 ) {
            // printf("Rec: %c\n", ch);
            oldTime = system_clock::now();

            // Special characters are not to be stored!
            if ( (ch < ' ' && ch != 0x09) || ch == '>' ) {
                // nl or cr means the line is done.
                // Take into account that it could start with one of those tho so check counts
                if ( rCnt > 0 && (ch == '\n' || ch == '\r') )
                    break;
                // Special case (looking at you, WT..)
                if ( noResp && ch == '>' ) {
                    rec[ rCnt ] = 0;
                    return true;
                }
            } else if ( rCnt > 0 || ch != ' ' || ch != 0x09 )  {
                if ( rCnt > (recSize - 2) ) {
                    printf("sendLine: Buffer too small for received line!");
                    return false;
                }

                rec[ rCnt++ ] = ch;
            }
        }

        newTime = system_clock::now();

        if ( duration_cast<milliseconds>(newTime - startTime).count() > 1000 ) {
            printf("sendLine took too long!\n");
            return false;
        }

    } while ( duration_cast<milliseconds>(newTime - oldTime).count() < 500 );

    // Terminate string
    rec[ rCnt ] = 0;

    // printf("%s\n", rec);

    return (rCnt > 0);
}

bool serial::sendLineMK2( const char *snd, char *rec, size_t recSize ) {
    bool escSeq = false;
    s32  ioStat, rCnt = 0;
    char ch;

    auto startTime = system_clock::now();
    auto lastRxTime = startTime;
    auto nowTime = startTime;

    // Only send if present and has data
    if ( snd != nullptr && snd[0] != 0 ) {
        size_t stLen = strlen( snd );
        // printf("sendLine sending: '%s'\n", snd);
        if ( stLen > 0 && (ioStat = write(sPort, snd, stLen)) != stLen ) {
            printf("Could not send line!\n");
            return false;
        }

        do {
            ioStat = read( sPort, &ch, 1 );

            // Device error
            if ( ioStat < 0 ) {
                printf("sendLine strip TX error\n");
                return false;
            }

            // Received something
            else if ( ioStat > 0 ) {
                // if ( ch >= ' ' && ch < 0x7f ) printf("sendLine strip: '%c' (%02x)\n", ch, ch);
                // else printf("sendLine strip: 'NaN' (%02x)\n", ch);

                // Update since last received
                lastRxTime = system_clock::now();

                // F*ck you ST firmware!
                if ( escSeq ) {
                    escSeq = false;
                    continue;
                } else if ( ch == 0x1b ) {
                    escSeq = true;
                    continue;
                }

                // If special character and _NOT_ tab, treat it as EOL
                if ( rCnt > 0 && ch < ' ' && ch != 0x09 )
                    goto startRx;
                rCnt++;
            }

            nowTime = system_clock::now();

            // Continously receiving crap. Just give up
            if ( duration_cast<milliseconds>(nowTime - startTime).count() > 4000 ) {
                printf("sendLine TX strip took too long!\n");
                return false;
            }

            // No more than 125 ms between consecutive chars
        } while ( duration_cast<milliseconds>(nowTime - lastRxTime).count() < 125 );

        printf("sendLine didn't see expected EOL of sent data\n");
        return false;
    }

startRx:
    rCnt = 0;
    escSeq = false;

    // Must contain at least 2 chars due to null-termination!
    if ( rec != nullptr && recSize > 1 ) {
        do {
            ioStat = read( sPort, &ch, 1 );


            // Device error
            if ( ioStat < 0 ) {
                printf("sendLine RX error\n");
                return false;
            }

            // Received something
            else if ( ioStat > 0 ) {
                // if ( ch >= ' ' && ch < 0x7f ) printf("sendLine RX: '%c' (%02x)\n", ch, ch);
                // else printf("sendLine RX: 'NaN' (%02x)\n", ch);

                // Update since last received
                lastRxTime = system_clock::now();

                // F*ck you ST firmware!
                if ( escSeq ) {
                    escSeq = false;
                    continue;
                } else if ( ch == 0x1b ) {
                    escSeq = true;
                    continue;
                }

                if ( ch < ' ' && ch != 0x09 ) {
                    if ( rCnt > 0 ) {
                        rec[ rCnt ] = 0;
                        return true;
                    }
                    // -- ignore char --
                } else {
                    rec[ rCnt++ ] = ch;
                    if ( rCnt >= recSize ) {
                        // We no longer have room for a terminating 0
                        printf("sendLine RX overflow!\n");
                        return false;
                    }
                }
            }

            nowTime = system_clock::now();

            // Continously receiving crap. Just give up
            if ( duration_cast<milliseconds>(nowTime - startTime).count() > 4000 ) {
                printf("sendLine took too long!\n");
                return false;
            }

            // No more than 125 ms between consecutive chars
        } while ( duration_cast<milliseconds>(nowTime - lastRxTime).count() < 125 );

        // There is at least one command that wants to know the prompt without EOL so rely on the RX timeout as a workaround
        if ( rCnt > 0 ) {
            rec[ rCnt ] = 0;
            return true;
        }
    } else {
        if ( rec != nullptr && recSize > 0 )
            rec[ 0 ] = 0;
        // Can't receive so assume everything went smoothly
        return true;
    }

    printf("sendLine timed out\n");

    return false;
}

bool serial::oneShot( const char *snd ) {

    s32 ioStat;
    size_t rCnt = 0;
    char ch;
    size_t stLen = strlen( snd );

    // printf("Sending: '%s'\n", snd);
    if ( (ioStat = write(sPort, snd, stLen)) != stLen ) {
        printf("Could not send line!\n");
        return false;
    }

    auto startTime = system_clock::now();

    // Strip up to and including sent message
    do {
        ioStat = read( sPort, &ch, 1 );
        // printf("strip: %c\n", ch);
        // Device error
        if ( ioStat < 0 ) {
            printf("sendLine strip error\n");
            return false;
        }
        // Received something
        else if ( ioStat > 0 ) {
            if ( ch == snd[ rCnt ] ) {
                if ( ++rCnt >= stLen )
                    break;
            } else {
                rCnt = 0;
            }
        }

        if ( duration_cast<milliseconds>(system_clock::now() - startTime).count() > 250 ) {
            printf("sendLine strip took too long!\n");
            return false;
        }
    } while ( 1 );

    return true;
}

bool serial::close() {
    if (sPort >= 0) {
        printf("Closing device\n");
        ::close(sPort);
    }
    sPort = -1;
    return true;
}

serial::~serial() {
    this->close();
}
