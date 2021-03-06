#include"IAPConfig.h"
#include"IAPStatic.h"
#include<IAP.h>
#include<STC15W4K48S4.h>
#include<string.h>

// A buffer is used to reduce wearing of flash storage
// MCU vender claimed it's EEPROM, but really it's just flash...
static unsigned char xdata sectorBuffer[512];
static unsigned char currentSector;

void IAP_init(){
    IAP_CONTR=0x80|SECTOR_ERASE_WAIT;
    currentSector=0;
    loadSector(0);
}

unsigned char IAP_read(unsigned int addr16){
    addr16%=ADDR_MAX;

    // If the target address is not loaded
    if(addr16/512!=currentSector){
        // Read from code region
        return *(unsigned char code*)(addr16+(unsigned int)MOVC_ADDR_OFFS);
    }else{
        // Read from buffer
        return sectorBuffer[addr16%512];
    }
}

unsigned char IAP_write(unsigned int addr16,unsigned char imm8){
    addr16%=ADDR_MAX;

    // If writing to an unloaded sector, unload current sector and load target sector
    if(addr16/512!=currentSector){
        unloadSector(currentSector);
        currentSector=addr16/512;
        loadSector(currentSector);
    }

    sectorBuffer[addr16%512]=imm8;
    return imm8;
}

void IAP_flush(){
    eraseSector(currentSector);
    writeSector(currentSector);
}

const unsigned char *IAP_getPtr(unsigned int addr16){
    addr16%=ADDR_MAX;

    if(addr16/512!=currentSector){
        return (const unsigned char code*)(addr16+(unsigned int)MOVC_ADDR_OFFS);
    }else{
        return sectorBuffer+addr16;
    }
}

static void loadSector(unsigned char sector){
    sector%=SECTOR_MAX;
    memcpy(sectorBuffer,(unsigned char code*)(sector*512+(unsigned int)MOVC_ADDR_OFFS),512);
}

static void unloadSector(unsigned char sector){
    sector%=SECTOR_MAX;

    eraseSector(sector);
    writeSector(sector);
}

static void eraseSector(unsigned char sector){
    sector%=SECTOR_MAX;

    IAP_CMD=SECTOR_ERASE;
    IAP_ADDRH=(sector*512)>>8;
    IAP_ADDRL=(sector*512);
    IAP_TRIG=TRIGGER_0;
    IAP_TRIG=TRIGGER_1;
}

static void writeSector(unsigned char sector){
    unsigned int i,addr=sector%SECTOR_MAX*512;

    // Program each byte in sector
    for(i=0;i<512;i++,addr++){
        IAP_DATA=sectorBuffer[i];
        IAP_CMD=BYTE_WRITE;
        IAP_ADDRH=addr>>8;
        IAP_ADDRL=addr;
        IAP_TRIG=TRIGGER_0;
        IAP_TRIG=TRIGGER_1;
    }
}
