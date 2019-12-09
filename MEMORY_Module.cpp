#include "EEPROM.h"

#define EEPROM_SIZE 64

void memory_init(void)
{
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("failed to initialise EEPROM");
    }
}

void memory_save_string (String data, int addr)
{
    if(addr == EEPROM_SIZE)
    {
        addr=0;
    }
    EEPROM.writeString(addr,data);
}

int memory_read_string (String * data, int addr)
{
    if(addr > EEPROM_SIZE) return 1;
    *data=EEPROM.readString(addr);
    return 0;
}