//correct finally fully completed alarm # multifunctional bed channel 2

#define F_CPU 8000000UL

#define D4 eS_PORTD4
#define D5 eS_PORTD5
#define D6 eS_PORTD6
#define D7 eS_PORTD7
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "hx711.h"

#include "I2C_Master_H_file.h"
#include "lcd.h"


#define KEY_PRT 	PORTA
#define KEY_DDR		DDRA
#define KEY_PIN		PINA

int hexadecimalToDecimal(char num1[]);
void alarm();
void display();
void fun1();
void fun2();
void fun3();

int alarmtimeout = 0;
double weight;
char printbuff[100];
/////////////////////////////////////////// KEYPAD ////////////////////////////////////////////////

unsigned char keypad[4][4] = {{'+','=','0','+'},	{'/','9','8','7'},
		{'*','6','5','4'},
				 {'-','3','2','1'}
				 };
	


unsigned char colloc, rowloc;

int userhour,userminute;


char keyfind()
{
	while(1)
	{
		KEY_DDR = 0xF0;           /* set port direction as input-output */
		KEY_PRT = 0xFF;

		do
		{
			KEY_PRT &= 0x0F;      /* mask PORT for column read only */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F); /* read status of column */
		}while(colloc != 0x0F);
		
		do
		{
			do
			{
				_delay_ms(20);             /* 20ms key debounce time */
				colloc = (KEY_PIN & 0x0F); /* read status of column */
				}while(colloc == 0x0F);        /* check for any key press */
				
				_delay_ms (40);	            /* 20 ms key debounce time */
				colloc = (KEY_PIN & 0x0F);
			}while(colloc == 0x0F);

			/* now check for rows */
			KEY_PRT = 0xEF;            /* check for pressed key in 1st row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 0;
				break;
			}

			KEY_PRT = 0xDF;		/* check for pressed key in 2nd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 1;
				break;
			}
			
			KEY_PRT = 0xBF;		/* check for pressed key in 3rd row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 2;
				break;
			}

			KEY_PRT = 0x7F;		/* check for pressed key in 4th row */
			asm("NOP");
			colloc = (KEY_PIN & 0x0F);
			if(colloc != 0x0F)
			{
				rowloc = 3;
				break;
			}
		}

		if(colloc == 0x0E)
		return(keypad[rowloc][0]);
		else if(colloc == 0x0D)
		return(keypad[rowloc][1]);
		else if(colloc == 0x0B)
		return(keypad[rowloc][2]);
		else
		return(keypad[rowloc][3]);
	}
	
	
	
	////////////////////////////////////////////// RTC //////////////////////////////////////////////////////////////////////
	
	#define Device_Write_address	0xD0				/* Define RTC DS1307 slave address for write operation */
	#define Device_Read_address		0xD1				/* Make LSB bit high of slave address for read operation */
	#define TimeFormat12			0x40				/* Define 12 hour format */
	#define AMPM					0x20


	int second,minute,hour;

// 	bool IsItPM(char hour_)
// 	{
// 		if(hour_ & (AMPM))
// 		return 1;
// 		else
// 		return 0;
// 	}

	void RTC_Read_Clock(char read_clock_address)
	{
		I2C_Start(Device_Write_address);				/* Start I2C communication with RTC */
		I2C_Write(read_clock_address);					/* Write address to read */
		I2C_Repeated_Start(Device_Read_address);		/* Repeated start with device read address */

		second = I2C_Read_Ack();						/* Read second */
		minute = I2C_Read_Ack();						/* Read minute */
		hour = I2C_Read_Nack();							/* Read hour with Nack */
		I2C_Stop();										/* Stop i2C communication */
	}

/////////////////////////////////////// Load Cell Calibration ////////////////////////////////////////////////////////

//defines running modes
#define HX711_MODERUNNING 1
#define HX711_MODECALIBRATION1OF2 2
#define HX711_MODECALIBRATION2OF2 3

#define HX711_MODECURRENT HX711_MODERUNNING

		//////////////////////// Steps to calibrate load cell ///////////////////////////////////////

//2 step calibration procedure
//set the mode to calibration step 1
//read the calibration offset leaving the load cell with no weight
//set the offset to value read
//put a know weight on the load cell and set calibrationweight value
//run the calibration status 2 of 2
//read the calibration scale
//set the scale to value read

//set the gain
int8_t gain = HX711_GAINCHANNELA128;

#if HX711_MODECURRENT == HX711_MODERUNNING
	//set the offset
	int32_t offset = 8391353;
	//set the scale
	double scale =841.176;
#elif HX711_MODECURRENT == HX711_MODECALIBRATION1OF2
	//set the offset
	int32_t offset = HX711_OFFSETDEFAULT;
	//set the scale
	double scale = HX711_SCALEDEFAULT;
#elif HX711_MODECURRENT == HX711_MODECALIBRATION2OF2
	//set the offset
	int32_t offset = 8391353;
	//set the scale
	double scale = HX711_SCALEDEFAULT;
	//set the calibration weight
	double calibrationweight = 0.170;
#endif

char numberpressed();

/////////////////////////////// main function /////////////////////////////////////////////////////////

int main(void) {
	char buffer[20];
	char buffer1[20];
	
	DDRD = 0xFF;
	DDRC = 0xFF;
	DDRB= 0xFF;
	PORTB |= 1<<PB7;
	
	sei();
	while(1){
	Lcd4_Init();
	I2C_Init();	
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("    WELCOME ");
	Lcd4_Set_Cursor(2,0);
	Lcd4_Write_String(" <<<<<<<>>>>>>>");
	_delay_ms(3000);
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("     HAVE A ");
	Lcd4_Set_Cursor(2,0);
	Lcd4_Write_String("    NICE DAY");
	_delay_ms(4000);


  char num[16];
  char num1[16];
  char num2[16];
  char num3[16];
  
	Lcd4_Clear();
	
	/*display();*/
	fun1();

	itoa(userhour,num1,10);
	itoa(userminute,num,10);
	
	
	int h = hexadecimalToDecimal(num1);
	int m = hexadecimalToDecimal(num);
	
	sprintf(buffer1, " Alarm  %02d:%02d",userhour,userminute);
	Lcd4_Clear();
	char printbuff[100];

	hx711_init(gain, scale, offset); //initiate hx711


#if HX711_MODECURRENT == HX711_MODERUNNING

	for(;;) {
		
		RTC_Read_Clock(0); //get read
		
		uint32_t read = hx711_read();
		ltoa(read, printbuff, 10);
		
		itoa(hour,num2,10);
		itoa(minute,num3,10);
		
		/*sprintf(buffer, "%02x:%02x:%02x  ", (hour & 0b00011111), minute,second);*/
		sprintf(buffer, "%02x:%02x", (hour & 0b00011111), minute);
		Lcd4_Clear();
		Lcd4_Set_Cursor(1,0);
		Lcd4_Write_String(buffer);
		Lcd4_Set_Cursor(2,0);
		Lcd4_Write_String(buffer1);
		
		//get weight
		double weight = hx711_getweight();
		
		////////////////////////////// SET Alarm ////////////////////////////////////
		DDRB &= ~(1<<PINB4);
		if((h == (hour & 0b00011111)) && (m == minute) && alarmtimeout == 0){


			
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,0);
				Lcd4_Write_String(" << ALARM ON >>");
				Lcd4_Set_Cursor(2,0);
				Lcd4_Write_String("TIME TO WAKE UP!");
				alarm();
				break;
		}
		
		_delay_ms(500);
	}
		
}

#elif HX711_MODECURRENT == HX711_MODECALIBRATION1OF2
for(;;) {
	//get calibration offset
	int32_t calibrationoffset = hx711_calibrate1getoffset();
	ltoa(calibrationoffset, printbuff, 10);
// 	Lcd4_Clear();
// 	Lcd4_Set_Cursor(1,0);
// 	Lcd4_Write_String("Calibration offset:  ");
// 	Lcd4_Set_Cursor(2,0);
// 	Lcd4_Write_String(printbuff);
// 	_delay_ms(1000);
}

#elif HX711_MODECURRENT == HX711_MODECALIBRATION2OF2
for(;;) {
	//calibrate
	hx711_calibrate2setscale(calibrationweight);

	//get scale
	double scale = hx711_getscale();
	dtostrf(scale, 3, 3, printbuff);
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("Calibration scale: ");
	Lcd4_Set_Cursor(2,0);
	Lcd4_Write_String(printbuff);
	_delay_ms(1000);
}
#endif
  
}

void alarm()
{
	while(1)
	{
		uint32_t read = hx711_read();
		ltoa(read, printbuff, 10);
		double weight = hx711_getweight();
// 		dtostrf(weight, 3, 3, printbuff);
// 		Lcd4_Clear();
// 		Lcd4_Set_Cursor(1,0);
// 		Lcd4_Write_String(" Weight (kg) : ");
// 		Lcd4_Set_Cursor(2,0);
// 		Lcd4_Write_String(printbuff);
		PORTB &= ~(1<<PB7);
		_delay_ms(100);
		PORTB |= 1<<PB7;
		_delay_ms(50);
		PORTB &= ~(1<<PB7);
		_delay_ms(100);
		PORTB |= 1<<PB7;
		_delay_ms(50);
		PORTB &= ~(1<<PB7);
		_delay_ms(100);
		PORTB |= 1<<PB7;
		_delay_ms(50);
		PORTB &= ~(1<<PB7);
		_delay_ms(50);
		PORTB |= 1<<PB7;
		_delay_ms(600);
		if (PINB & (1<<PINB4))
		{
			PORTB |= 1<<PB7;
			alarmtimeout = 1;
			break;
		}
		
		if(weight <= 3.0){
			PORTB |= 1<<PB7;
			alarmtimeout = 1;
			break;
		}
	}
}

void display(){
	char buffer[20];
	char c;
	
	char num[16];
	
	
	int i = 0;
	
	RTC_Read_Clock(0);
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	sprintf(buffer, "%02x:%02x", (hour & 0b00011111), minute);
	Lcd4_Write_String(buffer);
	_delay_ms(4000);
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("ALARM <<PRESS1");
	_delay_ms(4000);
	Lcd4_Clear();
	Lcd4_Set_Cursor(2,0);
	
	while(1){
		
		
		c = keyfind();
		
		
		if(c == '1'){
			if(i >= 1){
				Lcd4_Set_Cursor(2,0);
				Lcd4_Write_String(" Invalid ");
				_delay_ms(1000);
				display();
				break;
			}
			num[i] = c;
			i++;
			Lcd4_Write_Char(c);
			_delay_ms(1000);
			}else if(c == '*' ){
			fun1();
			break;
		}

	}
}

void fun1(){
	
	  char c;
	  
	  char num[16];
	  
	 alarmtimeout = 0;
	  int i = 0;
		

		Lcd4_Clear();
		Lcd4_Set_Cursor(1,0);
		Lcd4_Write_String(" Enter Hour");
		Lcd4_Set_Cursor(2,0);
		
		while(1){
			
			
		c = keyfind();
		
		
		if(c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0'){
			if(i >= 2){
				Lcd4_Set_Cursor(2,0);
				Lcd4_Write_String(" Invalid ");
				_delay_ms(1000);
				fun1();
				break;
			}
			num[i] = c;
			i++;
			Lcd4_Write_Char(c);
			_delay_ms(1000);
		}else if(c == '*' || c == '-'|| c == '/'|| c == '+'){
			userhour = atoi(num);
			fun2();
			break;
		}
		
		}
		
	
}


void fun2(){
	 char c;
	 
	 char num[16];
	 
	 
	 int i = 0;
	 

	 Lcd4_Clear();
	 Lcd4_Set_Cursor(1,0);
	 Lcd4_Write_String(" Enter Minutes");
	 Lcd4_Set_Cursor(2,0);
	 
	 while(1){
		 
		 
		 c = keyfind();
		 
		 
		 if(c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0'){
			 if(i >= 2){
				 Lcd4_Set_Cursor(2,0);
				 Lcd4_Write_String(" Invalid ");
				 _delay_ms(1000);
				 fun2();
				 break;
			 }
			 num[i] = c;
			 i++;
			 Lcd4_Write_Char(c);
			 _delay_ms(1000);
			 }else if(c == '*' || c == '-'|| c == '/'|| c == '+'){
				 userminute = atoi(num);
				 fun3();
			     break;
		 }

		 }
}

void fun3(){
	char c;
	
	char num[16];
	
	
	int i = 0;
	

	Lcd4_Clear();
	Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("PRESS 7>AM 8>PM");
	Lcd4_Set_Cursor(2,0);
	
	while(1){
		
		
		c = keyfind();
		
		
		if(c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0'){
			if(i >= 1){
				Lcd4_Set_Cursor(2,0);
				Lcd4_Write_String(" Invalid ");
				_delay_ms(1000);
				fun3();
				break;
			}
			num[i] = c;
			i++;
			Lcd4_Write_Char(c);
			_delay_ms(1000);
			break;
		}
		}

	}

// Function to convert hexadecimal to decimal
int hexadecimalToDecimal(char hexVal[])
{
	int len = strlen(hexVal);
	
	// Initializing base value to 1, i.e 16^0
	int base = 1;
	
	int dec_val = 0;
	
	// Extracting characters as digits from last character
	for (int i=len-1; i>=0; i--)
	{
		// if character lies in '0'-'9', converting
		// it to integral 0-9 by subtracting 48 from
		// ASCII value.
		if (hexVal[i]>='0' && hexVal[i]<='9')
		{
			dec_val += (hexVal[i] - 48)*base;
			
			// incrementing base by power
			base = base * 16;
		}
		
		// if character lies in 'A'-'F' , converting
		// it to integral 10 - 15 by subtracting 55
		// from ASCII value
		else if (hexVal[i]>='A' && hexVal[i]<='F')
		{
			dec_val += (hexVal[i] - 55)*base;
			
			// incrementing base by power
			base = base*16;
		}
	}
	
	return dec_val;
}

char numberpressed()
{
	PORTA = 0b10000000;
	if(PINA & (1<<PA0))
	{
		return 'a';
	}
	if(PINA & (1<<PA1))
	{
		return '3';
	}
	if(PINA & (1<<PA2))
	{
		return '2';
	}
	if(PINA & (1<<PA3))
	{
		return '1';
	}
	
	PORTA = 0b01000000;
	if(PINA & (1<<PA0))
	{
		return 'b';
	}
	if(PINA & (1<<PA1))
	{
		return '6';
	}
	if(PINA & (1<<PA2))
	{
		return '5';
	}
	if(PINA & (1<<PA3))
	{
		return '4';
	}
	PORTA = 0b00100000;
	if(PINA & (1<<PA0))
	{
		return 'c';
	}
	if(PINA & (1<<PA1))
	{
		return '9';
	}
	if(PINA & (1<<PA2))
	{
		return '8';
	}
	if(PINA & (1<<PA3))
	{
		return '7';
	}
	PORTA = 0b00010000;
	if(PINA & (1<<PA0))
	{
		return 'd';
	}
	if(PINA & (1<<PA1))
	{
		return '#';
	}
	if(PINA & (1<<PA2))
	{
		return '0';
	}
	if(PINA & (1<<PA3))
	{
		return '*';
	}
	
	return 'N';
}

