#include"LCD12864.h"
#include"LCD12864_ASCII5x8.h"

static unsigned char code
    CLEAR=0x01,
    STANDBY=0x01,
    HOME=0x02,
    RAM_ADDR_SELECT=0x02,
    ENTRY_MODE=0X04,
    DISPLAY_ON_OFF=0x08,
    CURSOR_DISPLAY_CONTROL=0x10,
    FUNCTION_SET=0x30,
    SET_CGRAM_ADDR=0x40,
    SET_DDRAM_ADDR=0x80,
    SET_GDRAM_ADDR=0x80;

static unsigned char code
    SPI_CPOL=1,
    SPI_CPHA=1,
    SPI_CLKDIV=3;

static unsigned char code
    BUFFER_INIT_VALUE=0x00,
    CHIP_SELECT_P2M0=0x40,
    CHIP_SELECT_P2M1=0x00;

static unsigned int code GDRAM_ADDRESS=0xf7ff;
static bit gdramNeedsFlush,gdramAccessLock=1;
static unsigned int brightness=0x0fff;

sbit chipSelect=P2^6;
sbit resetSignal=P2^7;

void lcd12864SpiSend(bit b,unsigned char c){
    spiSetup(SPI_CPOL,SPI_CPHA,SPI_CLKDIV);
    spiSetIsOccupied(1);

    chipSelect=1;

    spiSend(b?0xfa:0xf8);
    spiSend(c&0xf0);
    spiSend(c<<4);

    while(!spiTransmissionComplete());
    chipSelect=0;
    delayBusy(0,0,50);

    spiSetIsOccupied(0);
}

void lcd12864SpiSend2Bytes(bit b,unsigned char c1,unsigned char c2){
    spiSetup(SPI_CPOL,SPI_CPHA,SPI_CLKDIV);
    spiSetIsOccupied(1);

    chipSelect=1;

    spiSend(b?0xfa:0xf8);
    spiSend(c1&0xf0);
    spiSend(c1<<4);
    spiSend(b?0xfa:0xf8);
    spiSend(c2&0xf0);
    spiSend(c2<<4);

    while(!spiTransmissionComplete());
    chipSelect=0;
    delayBusy(0,0,50);

    spiSetIsOccupied(0);
}

void lcd12864HwReset(){
    gdramAccessLock=1;

    resetSignal=0;
    delayBusy(1,146,229);

    resetSignal=1;
    delayBusy(1,146,229);

    gdramAccessLock=0;
}

void lcd12864PwmInitialize(){
    pwm3Initialize(1,1,0,0,0,brightness);    // brightness
    pwmStart();
}

void brightnessSet(unsigned int brightness){
    if(brightness==0){
        brightness=1;
    }
    brightness=brightness;
    pwm3TimerValueSet(0,brightness);
}

unsigned int brightnessGet(){
    return brightness;
}

void lcd12864FlushAtNextUpdate(){
    gdramNeedsFlush=1;
}

void lcd12864SpiInitialize(){
    unsigned char i,j;

    chipSelect=0;
    P2M0=CHIP_SELECT_P2M0;
    P2M1=CHIP_SELECT_P2M1;

    lcd12864HwReset();
    lcd12864SpiSend(0,FUNCTION_SET|0x04);
    for(i=0;i<32;i++){
        lcd12864SpiSend2Bytes(0,SET_GDRAM_ADDR|i,SET_GDRAM_ADDR);
        for(j=0;j<32;j+=2){
            lcd12864SpiSend2Bytes(1,BUFFER_INIT_VALUE,BUFFER_INIT_VALUE);
        }
    }
    lcd12864SpiSend(0,FUNCTION_SET|0x04|0x02);

    lcd12864PwmInitialize();
    i23lc512Memset(GDRAM_ADDRESS,BUFFER_INIT_VALUE,32*32*2);

    gdramAccessLock=0;
}

void lcd12864GdramFlush(bit forceFlush){
    unsigned char buffer[64];
    unsigned int i,j;
    bit graphicDisplayDisabled=0,addressJustBeenSent=0;

    if(gdramNeedsFlush&&!gdramAccessLock){
        for(i=0;i<32;i++){
            i23lc512UCharSeqRead(buffer,GDRAM_ADDRESS+64*i,64);

            for(j=0;j<32;j+=2){
                if(forceFlush||buffer[j+0]!=buffer[j+32]||buffer[j+1]!=buffer[j+33]){
                    if(!graphicDisplayDisabled){
                        graphicDisplayDisabled=1;
                        lcd12864SpiSend(0,FUNCTION_SET|0x04);
                    }

                    buffer[j+0]=buffer[j+32];
                    buffer[j+1]=buffer[j+33];

                    if(1||!addressJustBeenSent){    // ???
                        lcd12864SpiSend2Bytes(0,SET_GDRAM_ADDR|i,SET_GDRAM_ADDR|j/2);
                        addressJustBeenSent=1;
                    }

                    lcd12864SpiSend2Bytes(1,buffer[j+0],buffer[j+1]);
                }else{
                    addressJustBeenSent=0;
                }
            }
            i23lc512UCharSeqWrite(buffer,GDRAM_ADDRESS+64*i,64);
        }

        if(graphicDisplayDisabled){
            lcd12864SpiSend(0,FUNCTION_SET|0x04|0x02);
        }
        gdramNeedsFlush=0;
    }
}

void lcd12864CharSet(unsigned char row,unsigned char col,unsigned char c,bit flush){
    unsigned char i,buffer[32];
    gdramAccessLock=1;
    gdramNeedsFlush|=flush;

    col=col%25*5;
    row=row%8*8;
    col+=(row>31)*128;
    row%=32;

    for(i=row;i<row+8;i++){
        i23lc512UCharSeqRead(buffer,GDRAM_ADDRESS+64*i+32,32);

        buffer[col/8]&=(0xff<<(8-col%8));
        buffer[col/8]|=(ASCII5x8[c][i%8]>>(col%8));
        buffer[col/8+1]&=(0xff>>(col%8));
        buffer[col/8+1]|=(ASCII5x8[c][i%8]<<(8-col%8));

        i23lc512UCharSeqWrite(buffer,GDRAM_ADDRESS+64*i+32,32);
    }

    gdramAccessLock=0;
}

void lcd12864StringSet(unsigned char row,unsigned char col,unsigned char *str,bit flush){
    unsigned char i,j,k,buffer[32];
    gdramAccessLock=1;
    gdramNeedsFlush|=flush;

    col=col%25*5;
    row=row%8*8;
    col+=(row>31)*128;
    row%=32;

    for(i=row;i<row+8;i++){
        i23lc512UCharSeqRead(buffer,GDRAM_ADDRESS+64*i+32,32);
        k=col;
        for(j=0;str[j];j++){
            buffer[k/8]&=(0xff<<(8-k%8));
            buffer[k/8]|=(ASCII5x8[str[j]][i%8]>>(k%8));
            buffer[k/8+1]&=(0xff>>(k%8));
            buffer[k/8+1]|=(ASCII5x8[str[j]][i%8]<<(8-k%8));
            k+=5;
        }
        i23lc512UCharSeqWrite(buffer,GDRAM_ADDRESS+64*i+32,32);
    }

    gdramAccessLock=0;
}
