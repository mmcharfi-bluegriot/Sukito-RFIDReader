#include "EEPROM.h"

#define EEPROM_SIZE 64

void memory_init(void);
void memory_save_string (String data, int addr);
int memory_read_string (String * data, int addr);
void memory_send_save();