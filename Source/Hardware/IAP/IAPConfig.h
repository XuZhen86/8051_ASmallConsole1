#ifndef IAPCONFIG_H_
#define IAPCONFIG_H_

// MCU-specific data
enum IAP_CONFIG{
    SECTOR_MAX=20,
    SECTOR_ERASE_WAIT=0x0,
    MOVC_ADDR_OFFS=0xc000,
    ADDR_MAX=0x2800,
};

enum IAP_COMMAND{
    STANDBY=0x0,
    BYTE_READ=0x1,
    BYTE_WRITE=0x2,
    SECTOR_ERASE=0x3,
    TRIGGER_0=0x5a,
    TRIGGER_1=0xa5,
};

#endif
