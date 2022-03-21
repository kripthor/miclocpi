#include "miclocpi.h"
#include "minimal_gpio.c"
#include "ds1077.c"
#include "signal_processing.c"
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

#define AVGWINDOW 32
void processSamples(){
	int i;

	for (i=0;i<BANK_SIZE-AVGWINDOW;i++) {
		bank1[i] = avg(bank1+i,AVGWINDOW);
		bank2[i] = avg(bank2+i,AVGWINDOW);
		bank3[i] = avg(bank3+i,AVGWINDOW);
		bank4[i] = avg(bank4+i,AVGWINDOW);
	//	bank5[i] = avg(bank5+i,AVGWINDOW);
	//	bank6[i] = avg(bank6+i,AVGWINDOW);
	}
	
	cc12 = crossCorrelation2(bank1,bank2,9000);
	cc13 = crossCorrelation2(bank1,bank3,9000);
	cc14 = crossCorrelation2(bank1,bank4,9000);
	printf("CC12: %d\n",cc12);
	printf("CC13: %d\n",cc13);
	printf("CC14: %d\n",cc14);
		
}


#define SKIP_SAMPLES 0
#define CHECK_SAMPLES SKIP_SAMPLES+8
#define THRESHOLD 50
//midpoint should be 127 in perfect conditions
#define MIDPOINT 140

#define DELAYSAMPLES 100
#define DELAYDETECT 200

 
int detectSound() {
	unsigned char sample;
	int aboveThreshold = 0;
	
	DAS_Reset();
	DAS_Enable();
	//delayMicroseconds(50);
	
	//sample till half
	//sampleHalfFull();
	//get some samples on mic1
	
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



void oled(char *str) {
	char cmd[256];
	//no command injection protection
	snprintf(cmd,256,"sudo python strToOled.py %s&",str);
	system(cmd);
}



int main(void){
	u_int32_t start,stop;
	
	if (gpioInitialise() < 0) return -1;
	wiringPiSetupGpio();
	setupGpios();
	piHiPri(99);
	setDS1077FreqDiv(11); //10Mhz
	//setDS1077FreqDiv(6); //16Mhz
	
	/*
	int kk = 0;
	for (kk = 2; kk < 20; kk++) {
		setDS1077FreqDiv(kk);
		start = gpioTick();
		sampleFull();
		stop = gpioTick();
		printf("Sample(1) took(us): %d\n",(stop-start));
		printf("DIV %d - Mhz: %f\n",kk,BANK_SIZE*1.0/(stop-start));
	}
	return 0;
	*/
	
	
	start = gpioTick();
	sampleFull();
	stop = gpioTick();
	printf("sample(1) took(ms): %f\n",(stop-start)/1000.0);
	printf("Mhz: %f\n",BANK_SIZE*1.0/(stop-start));


	start = gpioTick();
	getSamples();
	stop = gpioTick();
	printf("getSamples() took(ms): %f\n",(stop-start)/1000.0);

	DAS_Enable();
	start = gpioTick();
	detectSound();
	stop = gpioTick();
	printf("detectSound() took(ms): %f\n",(stop-start)/1000.0);


	oled("'Waiting for sound...'");
	DAS_Enable();
	while (detectSound() < CHECK_SAMPLES/2) {
	}	
		
	oled("'Sound detected...' 20");
	waitForFull();
	getSamples();
	oled("'Sound processing...' 30");
	

	start = gpioTick();
	processSamples();
	stop = gpioTick();
	printf("processSamples() took(ms): %f\n",(stop-start)/1000.0);
	

	int k;
//	for (k=0;k<BANK_SIZE-AVGWINDOW;k++) {
//		printf("%d;%d;%d;%d;%d;%d\n",bank1[k],bank2[k],bank3[k],bank4[k],bank5[k],bank6[k]);
//	}
	for (k=0;k<BANK_SIZE-AVGWINDOW;k++) {
		printf("%d;%d;%d;%d\n",bank1[k],bank2[k],bank3[k],bank4[k]);
	}
	
	return 0;
}







