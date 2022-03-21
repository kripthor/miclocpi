#include "ds1077.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <wiringPi.h>

//http://datasheets.maximintegrated.com/en/ds/DS1077.pdf

int setDS1077FreqDiv(unsigned int i)
{	
	///TODO return error codes
	
	int deviceHandle;
	unsigned char buffer[3];
  
	/// ACCESS MUX COMMAND (0x02), next data is writen to MUX register, to set prescaler as we want
	// open device on /dev/i2c-1
	deviceHandle = open("/dev/i2c-1", O_RDWR);
	// connect to arduino as i2c slave
	ioctl(deviceHandle, I2C_SLAVE, DS1077_ADDR);
	// begin transmission and request acknowledgement
	 // initialize buffer
	memset(buffer,0x00,4);
	buffer[0] = 0x02;
	buffer[1] = ( 0
        | (MUX_HI_PDN1_BIT & BIT_OFF)
        | (MUX_HI_PDN0_BIT & BIT_OFF)
        | (MUX_HI_SEL0_BIT & BIT_ON)
        | (MUX_HI_EN0_BIT  & BIT_ON)
        | (MUX_HI_0M1_BIT  & BIT_OFF) //PRESCALER P0 High bit
        | (MUX_HI_0M0_BIT  & BIT_OFF) //PRESCALER P0 Low bit
        | (MUX_HI_1M1_BIT  & BIT_OFF) //PRESCALER P1 High bit
      );
	buffer[2] = ( 0
        | (MUX_LO_1M0_BIT  & BIT_OFF) //PRESCALER P1 Low bit
        | (MUX_LO_DIV1_BIT & BIT_OFF)
      );
	write(deviceHandle, buffer, 3);
	// close connection and return
	close(deviceHandle);
	delay(20);
	
    /// ACCESS DIV COMMAND (0x01), next data is writen to MUX register
	// open device on /dev/i2c-1
	deviceHandle = open("/dev/i2c-1", O_RDWR);
	// connect to arduino as i2c slave
	ioctl(deviceHandle, I2C_SLAVE, DS1077_ADDR);
	// begin transmission and request acknowledgement
	 // initialize buffer
	memset(buffer,0x00,4);
	buffer[0] = 0x01;
	buffer[1] = i>>2;
	buffer[2] = (i & 0x3)<<6;
	write(deviceHandle, buffer, 3);
	// close connection and return
	close(deviceHandle);
	delay(20);

    /// WRITE CHANGES TO EEPROM COMMAND (0x3F)
	// open device on /dev/i2c-1
	deviceHandle = open("/dev/i2c-1", O_RDWR);
	// connect to arduino as i2c slave
	ioctl(deviceHandle, I2C_SLAVE, DS1077_ADDR);
	// begin transmission and request acknowledgement
	// initialize buffer
	memset(buffer,0x00,4);
	buffer[0] = 0x3F;
	write(deviceHandle, buffer, 1);
	// close connection and return
	close(deviceHandle);
	
	return 0;
    
}
