#include "miclocpi.h"
#include "minimal_gpio.c"
#include "ds1077.c"
#include <wiringPi.h>


unsigned char bank1[BANK_SIZE];
unsigned char bank2[BANK_SIZE];
unsigned char bank3[BANK_SIZE];
unsigned char bank4[BANK_SIZE];
unsigned char bank5[BANK_SIZE];
unsigned char bank6[BANK_SIZE];

int cc12,cc13,cc14,cc15,cc16;


// DOES THIS IRQ DISABLE ACTUALLY WORK?
void disable_interrupts()
{
  __asm__("CPSID I\n"); 
}
void enable_interrupts()
{
  __asm__("CPSIE I\n"); 
}



void DAS_Enable() {
  gpioWrite(DAS_ENABLE,LOW);
}

void DAS_Disable() {
  gpioWrite(DAS_ENABLE,HIGH);
}


void DAS_Reset() {
	gpioWrite(DAS_READ_BANK1,HIGH); 
	gpioWrite(DAS_READ_BANK2,HIGH); 
	gpioWrite(DAS_READ_BANK3,HIGH); 
	gpioWrite(DAS_READ_BANK4,HIGH); 
	gpioWrite(DAS_READ_BANK5,HIGH); 
	gpioWrite(DAS_READ_BANK6,HIGH); 
	gpioWrite(DAS_RESET,LOW);
	//delayMicroseconds(1);
	DAS_Enable();
	//delayMicroseconds(2);
	DAS_Disable();
	gpioWrite(DAS_RESET,HIGH); 
}



void setupGpios() {

	gpioSetMode(DAS_ENABLE,PI_OUTPUT);
	gpioSetMode(DAS_RESET,PI_OUTPUT);
	gpioSetMode(DAS_HFULL,PI_INPUT);
	gpioSetMode(DAS_FULL,PI_INPUT);
  
	gpioSetMode(DAS_READ_BANK1,PI_OUTPUT);
	gpioSetMode(DAS_READ_BANK2,PI_OUTPUT);
	gpioSetMode(DAS_READ_BANK3,PI_OUTPUT);
	gpioSetMode(DAS_READ_BANK4,PI_OUTPUT);
	gpioSetMode(DAS_READ_BANK5,PI_OUTPUT);
	gpioSetMode(DAS_READ_BANK6,PI_OUTPUT);

	//PORTA Input
	gpioSetMode(DAS_BIT0,PI_INPUT);
	gpioSetMode(DAS_BIT1,PI_INPUT);
	gpioSetMode(DAS_BIT2,PI_INPUT);
	gpioSetMode(DAS_BIT3,PI_INPUT);
	gpioSetMode(DAS_BIT4,PI_INPUT);
	gpioSetMode(DAS_BIT5,PI_INPUT);
	gpioSetMode(DAS_BIT6,PI_INPUT);
	gpioSetMode(DAS_BIT7,PI_INPUT);
  
	DAS_Disable();
	DAS_Reset();

}

void sampleFull() {
	
	DAS_Reset();
	DAS_Enable();
	disable_interrupts();
	while(gpioRead(DAS_FULL)) { };
	enable_interrupts();	
	
}

void sampleHalfFull() {
	
	DAS_Reset();
	DAS_Enable();
	disable_interrupts();
	while(gpioRead(DAS_HFULL)) { };
	enable_interrupts();	
	
}

void waitForFull() {
	
	disable_interrupts();
	while(gpioRead(DAS_FULL)) { };
	enable_interrupts();	
	
}

void customDelay(volatile int x) {
	//experimental results show lower value than 40 is too fast and introduces many reading errors. 80 is more safe
	//this is mainly because of the wiring in the hardware, proper hardware should support the full 40Mhz shift freq
	//volatile is needed because of gcc optimization
	while (x--);
}


#define SKIP_SAMPLES 32
#define CHECK_SAMPLES SKIP_SAMPLES+32
#define THRESHOLD 60 
//midpoint should be 127 in perfect conditions
#define MIDPOINT 127
#define DELAYSAMPLES 80
#define DELAYDETECT 200

 
int detectSound() {
	unsigned char sample;
	int aboveThreshold = 0;
	
	DAS_Reset();
	DAS_Enable();
	
	//samples are enabled and entering faster than can be read (assuming high enough sample rate),
	//it's safe to read away
	
	unsigned int i;
	  
	for (i=0;i<CHECK_SAMPLES;i++) {
        gpioWrite(DAS_READ_BANK1,LOW); 
        customDelay(DELAYDETECT);
        sample = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK1,HIGH);
        customDelay(DELAYDETECT);
        if (i > SKIP_SAMPLES) {
			if (sample > MIDPOINT+THRESHOLD || sample < MIDPOINT-THRESHOLD) aboveThreshold++;
		}
		if (sample > 250 || sample < 5) aboveThreshold--;
		gpioWrite(DAS_READ_BANK2,LOW); 
        customDelay(DELAYDETECT);
        sample = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK2,HIGH);
        customDelay(DELAYDETECT);
        if (i > SKIP_SAMPLES) {
			if (sample > MIDPOINT+THRESHOLD || sample < MIDPOINT-THRESHOLD) aboveThreshold++;
		}
		if (sample > 250 || sample < 5) aboveThreshold--;
		gpioWrite(DAS_READ_BANK3,LOW); 
        customDelay(DELAYDETECT);
        sample = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK3,HIGH);
        customDelay(DELAYDETECT);
        if (i > SKIP_SAMPLES) {
			if (sample > MIDPOINT+THRESHOLD || sample < MIDPOINT-THRESHOLD) aboveThreshold++;
		}
		if (sample > 250 || sample < 5) aboveThreshold--;
		gpioWrite(DAS_READ_BANK4,LOW); 
        customDelay(DELAYDETECT);
        sample = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK4,HIGH);
        customDelay(DELAYDETECT);
        if (i > SKIP_SAMPLES) {
			if (sample > MIDPOINT+THRESHOLD || sample < MIDPOINT-THRESHOLD) aboveThreshold++;
		}
		if (sample > 250 || sample < 5) aboveThreshold--;
    }

	return aboveThreshold;
}



void getSamples(){
	DAS_Disable();
	unsigned int i;
	  
	for (i=0;i<BANK_SIZE;i++) {

        gpioWrite(DAS_READ_BANK1,LOW); 
        customDelay(DELAYSAMPLES);
        bank1[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK1,HIGH);
        customDelay(DELAYSAMPLES);
        
        gpioWrite(DAS_READ_BANK2,LOW); 
        customDelay(DELAYSAMPLES);
        bank2[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK2,HIGH);
        customDelay(DELAYSAMPLES);
        
        gpioWrite(DAS_READ_BANK3,LOW); 
        customDelay(DELAYSAMPLES);
        bank3[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK3,HIGH);
        customDelay(DELAYSAMPLES);

        gpioWrite(DAS_READ_BANK4,LOW); 
        customDelay(DELAYSAMPLES);
        bank4[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK4,HIGH);
        customDelay(DELAYSAMPLES);

        /* MIC 5/6 are disabled
        gpioWrite(DAS_READ_BANK5,LOW); 
        customDelay();
        bank5[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK5,HIGH);
        customDelay();

        gpioWrite(DAS_READ_BANK6,LOW); 
        customDelay();
        bank6[i] = (gpioReadBank1()&0x00ff0000)>>16;
        gpioWrite(DAS_READ_BANK6,HIGH);
        customDelay();
		*/
      }

    
}




int main(void){
	u_int32_t start,stop;
	
	if (gpioInitialise() < 0) return -1;
	wiringPiSetupGpio();
	setupGpios();
	//piHiPri(90);
//	setDS1077FreqDiv(6); //16Mhz
	setDS1077FreqDiv(11); //10Mhz
	//setDS1077FreqDiv(64); //2Mhz
	
	start = gpioTick();
	sampleFull();
	stop = gpioTick();
	printf("#sample(1) took(ms): %f - ",(stop-start)/1000.0);
	printf("Mhz: %f\n",BANK_SIZE*1.0/(stop-start));

	int goodSamples = 0;
	
	while (!goodSamples) {
		DAS_Enable();
		while (detectSound() < CHECK_SAMPLES-SKIP_SAMPLES);	
		
		//Either sampleFull (loose some time reseting and so on or wait for full)
		//Wait for full might loose some sample at the end, up to a maximum of CHECK_SAMPLES+SKIP_SAMPLES, which usually is ok
		waitForFull();
		start = gpioTick();
		getSamples();

		int k;

		#define CLIPLIMIT 3
		#define MAXCLIPS 200+CHECK_SAMPLES*4
		int clipped = 0;
		for (k=0;k<BANK_SIZE;k++) {
			if (bank1[k] > 255 - CLIPLIMIT || bank1[k] < 0 + CLIPLIMIT) clipped++;
			if (bank2[k] > 255 - CLIPLIMIT || bank2[k] < 0 + CLIPLIMIT) clipped++;
			if (bank3[k] > 255 - CLIPLIMIT || bank3[k] < 0 + CLIPLIMIT) clipped++;
			if (bank4[k] > 255 - CLIPLIMIT || bank4[k] < 0 + CLIPLIMIT) clipped++;
		}
		if (clipped < MAXCLIPS) goodSamples = 1;
		if (goodSamples) {
			for (k=0;k<BANK_SIZE;k++) {
				printf("%03d;%03d;%03d;%03d\n",bank1[k],bank2[k],bank3[k],bank4[k]);
			}
		}
		else {
			fprintf(stderr,"Too much clipping, %d",clipped);
		}
	}
	return 0;
}







