/**************************************************************************/
/*!
    @file     GSL2038.h

    @thanks to Skallwar for the source code this lib is bassed on : https://github.com/Skallwar/GSL1680
*/

#ifndef GSL2038_H
#define GSL2038_H

#include <stdint.h>

class GSL2038 {
    public:
        GSL2038();
        GSL2038(bool error, bool info);

        void begin(uint8_t WAKE, uint8_t INTRPT);
        uint8_t dataread();
        uint8_t readFingerID(int NB);
        uint32_t readFingerX(int NB);
        uint32_t readFingerY(int NB);

    private:
        bool GSL2038_DEBUG_ERROR;
        bool GSL2038_DEBUG_INFO;
        void reset();
        void loadfw();
        void startchip();
        void sleep();
        void datasend(uint8_t REG, uint8_t DATA[], uint16_t NB);
};

struct Finger {
    uint8_t fingerID;
    uint32_t x;
    uint32_t y;
};

struct Touch_event {
    uint8_t NBfingers;
    struct Finger fingers[5];
};

#endif
