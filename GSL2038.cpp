/**************************************************************************/
/*!
    @file     GSL2038.h

    @thanks to Skallwar for the source code this lib is bassed on : https://github.com/Skallwar/GSL2038
*/

#include <Arduino.h>
#include <Wire.h>

#include <stdint.h>

#include "GSL2038.h"
#include "gsl2038firmware.h"

// Registres
#define I2CADDR 0x40
#define DATA_REG 0x80
#define STATUS_REG 0xE0
#define PAGE_REG 0xF0

//CONST
#define BUFSIZE 32

#define SERIAL_ERROR if(GSL2038_DEBUG_ERROR)Serial
#define SERIAL_INFORMATION if(GSL2038_DEBUG_INFO)Serial

struct Touch_event ts_event;

GSL2038::GSL2038() {
    GSL2038_DEBUG_ERROR = false;
    GSL2038_DEBUG_INFO = false;
}

GSL2038::GSL2038(bool error, bool info) {
    GSL2038_DEBUG_ERROR = error;
    GSL2038_DEBUG_INFO = info;
}

void GSL2038::begin(uint8_t WAKE, uint8_t INTRPT)
{
    SERIAL_INFORMATION.println("GSL2038: Start boot up sequence");
    pinMode(WAKE, OUTPUT);          //
    digitalWrite(WAKE, LOW);        //
    pinMode(INTRPT, INPUT_PULLUP);  // Startup sequence PIN part
    delay(100);

    SERIAL_INFORMATION.println("Toggle Wake");
	digitalWrite(WAKE, HIGH);
	delay(50);
	digitalWrite(WAKE, LOW);
	delay(50);
	digitalWrite(WAKE, HIGH);
	delay(30);

    // CTP startup sequence
	SERIAL_INFORMATION.println("Reset chip");
	reset();
	SERIAL_INFORMATION.println("GSL2038: Load FW");
	loadfw();
	//startup_chip();
	SERIAL_INFORMATION.println("Reset chip 2");
	reset();
	SERIAL_INFORMATION.println("Startup Chip");
    startchip();
	SERIAL_INFORMATION.println("GSL2038: Boot up complete");
}


void GSL2038::reset()
{
    uint8_t REG[6] = {STATUS_REG, 0xE4, 0xbc, 0xbd, 0xbe, 0xbf};
    uint8_t DATA[6] = {0x88, 0x04, 0x00, 0x00, 0x00, 0x00};
    uint8_t TIMER[6] = {10, 10, 10, 10, 10, 10};

    int i;
    for (i = 0; i < sizeof(REG); ++i)
    {
        Wire.beginTransmission(I2CADDR);

        Wire.write(REG[i]);
        Wire.write(DATA[i]);
        delay(TIMER[i]);

        int r = Wire.endTransmission();
        if (r != 0)
        {
            SERIAL_ERROR.print("i2c write error: ");
            SERIAL_ERROR.print(r);
            SERIAL_ERROR.print(" ");
            SERIAL_ERROR.println(REG[i], HEX);
        }
    }
}

void GSL2038::loadfw()
{
    uint8_t addr;
    uint8_t Wrbuf[4];
    size_t source_len = sizeof(GSL2038_FW) / sizeof(struct fw_data);

    for (size_t source_line = 0; source_line < source_len; source_line++) {
        addr = GSL2038_FW[source_line].offset;
        Wrbuf[0] = (char)(GSL2038_FW[source_line].val & 0x000000ff);
        Wrbuf[1] = (char)((GSL2038_FW[source_line].val & 0x0000ff00) >> 8);
        Wrbuf[2] = (char)((GSL2038_FW[source_line].val & 0x00ff0000) >> 16);
        Wrbuf[3] = (char)((GSL2038_FW[source_line].val & 0xff000000) >> 24);

        datasend(addr, Wrbuf, 4);
    }
}

void GSL2038::startchip()
{
    Wire.beginTransmission(I2CADDR);

    Wire.write(0xE0); // Registre
    Wire.write(0x00); // DATA

    int r = Wire.endTransmission();
    if (r != 0)
    {
        SERIAL_ERROR.print("i2c write error: ");
        SERIAL_ERROR.print(r);
        SERIAL_ERROR.print(" ");
        SERIAL_ERROR.println(0xE0, HEX);
    }
}

void GSL2038::sleep()
{

}

void GSL2038::datasend(uint8_t REG, uint8_t DATA[], uint16_t NB)
{
    Wire.beginTransmission(I2CADDR);

    Wire.write(REG);

    for (uint16_t i = 0; i < NB; i++)
    {
        Wire.write(DATA[i]);
    }

    int r = Wire.endTransmission();
    if (r != 0)
    {
        SERIAL_ERROR.print("i2c write error: ");
        SERIAL_ERROR.print(r);
        SERIAL_ERROR.print(" ");
        SERIAL_ERROR.println(REG, HEX);
    }
}

uint8_t GSL2038::dataread()
{
    uint8_t TOUCHRECDATA[24] = {0};

    Wire.beginTransmission(I2CADDR);

    Wire.write(DATA_REG);

    int n = Wire.endTransmission();
    if (n != 0)
    {
        SERIAL_ERROR.print("i2c write error: ");
        SERIAL_ERROR.print(n);
        SERIAL_ERROR.print(" ");
        SERIAL_ERROR.println(DATA_REG, HEX);
    }

    n = Wire.requestFrom(I2CADDR, 24);
    if (n != 24)
    {
        SERIAL_ERROR.print("i2c read error: did not get expected count ");
        SERIAL_ERROR.print(n);
        SERIAL_ERROR.print("/");
        SERIAL_ERROR.println("24");
    }

    for (int i = 0; i < n; i++)
    {
        TOUCHRECDATA[i] = Wire.read();
    }

    ts_event.NBfingers = TOUCHRECDATA[0];
    for (int i = 0; i < ts_event.NBfingers; i++)
    {
        ts_event.fingers[i].x = ((((uint32_t)TOUCHRECDATA[(i * 4) + 5]) << 8) | (uint32_t)TOUCHRECDATA[(i * 4) + 4]) & 0x00000FFF; // 12 bits of X coord
        ts_event.fingers[i].y = ((((uint32_t)TOUCHRECDATA[(i * 4) + 7]) << 8) | (uint32_t)TOUCHRECDATA[(i * 4) + 6]) & 0x00000FFF;
        ts_event.fingers[i].fingerID = (uint32_t)TOUCHRECDATA[(i * 4) + 7] >> 4; // finger that did the touch
    }

    return ts_event.NBfingers;
}

uint8_t GSL2038::readFingerID(int NB)
{
    return ts_event.fingers[NB].fingerID;
}

uint32_t GSL2038::readFingerX(int NB)
{
    return ts_event.fingers[NB].x;
}

uint32_t GSL2038::readFingerY(int NB)
{
    return ts_event.fingers[NB].y;
}
