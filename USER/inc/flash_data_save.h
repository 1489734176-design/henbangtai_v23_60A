#ifndef __FLASH_DATA_SAVE_H
#define __FLASH_DATA_SAVE_H
#include <stdio.h>
#include "mm32_device.h"
#include "hal_conf.h"



//void write_half_data(uint32_t Address,uint16_t write_data);
uint16_t read_half_data(uint32_t Address);
void save_data_to_flash(uint16_t* save_data,uint8_t length );
u8 read_data_for_flash(uint16_t *read_data,uint8_t length);
void save_user_data(void);
void read_user_data(void);
#endif
