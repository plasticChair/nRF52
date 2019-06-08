#ifndef GLOBALTYPE_H
#define GLOBALTYPE_H


#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"


struct SD_Card_Struct {
    FATFS fs;
    DIR dir;
    FILINFO fno;
    FIL file;

    uint32_t bytes_written;
    uint32_t bytes_read;
    FRESULT ff_result;
    DSTATUS disk_state;

};


#endif /* MINMEA_H */