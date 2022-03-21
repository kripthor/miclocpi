//-------------------------------------------------
// external clock source via I2C
//DS1077 default address
#define DS1077_ADDR 0x58

#define MUX_HI_PDN1_BIT 0x01<<6
#define MUX_HI_PDN0_BIT 0x01<<5
#define MUX_HI_SEL0_BIT 0x01<<4
#define MUX_HI_EN0_BIT  0x01<<3
#define MUX_HI_0M1_BIT  0x01<<2
#define MUX_HI_0M0_BIT  0x01<<1
#define MUX_HI_1M1_BIT  0x01

#define MUX_LO_1M0_BIT  0x01<<7
#define MUX_LO_DIV1_BIT 0x01<<6

#define BIT_ON (0xff)
#define BIT_OFF (0x00)
