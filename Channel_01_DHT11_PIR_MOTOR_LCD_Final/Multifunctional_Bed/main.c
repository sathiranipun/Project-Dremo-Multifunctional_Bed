/*
 * Multifunctional_Bed.c
 *
 * Created: 5/25/2019 4:43:32 PM
 * Author : SATHIRA
 */ 

#define F_CPU 8000000UL
#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
//#include <avr/interrupt.h>

#include "i2cmaster.h"
#include "i2c_lcd.h"
#define DHT11_PIN 5 // PD5

void Request();
void Response();
uint8_t Receive_data();

// Relay Module is Ground Trigger

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DHT11 TEMP SENSOR

uint8_t c=0,I_RH,D_RH,I_Temp,D_Temp,CheckSum;

void Request()				// Microcontroller send start pulse/request 
{
	DDRD |= (1<<DHT11_PIN);
	PORTD &= ~(1<<DHT11_PIN);	
	_delay_ms(18);	
	PORTD |= (1<<DHT11_PIN);	
	
}

void Response()				/* receive response from DHT11 */
{
	DDRD &= ~(1<<DHT11_PIN);
	while(PIND & (1<<DHT11_PIN));
	while((PIND & (1<<DHT11_PIN))==0);
	while(PIND & (1<<DHT11_PIN));
	
}

uint8_t Receive_data()			/* receive data */
{
	for (int q=0; q<8; q++)
	{
		while((PIND & (1<<DHT11_PIN)) == 0);  
		_delay_us(30);
		if(PIND & (1<<DHT11_PIN))
		c = (c<<1)|(0x01);	
		else			
		c = (c<<1);
		while(PIND & (1<<DHT11_PIN));
	}
	return c;
}

/////////////////////////////////////////// Initiating Phase ////////////////////////////////////////////////////////////////////

int main(void)
{
	
	//////////////////////////// SET Data Pin Direction ////////////////////////////////////////////////////////////////
	DDRA &= ~(1<<PINA0);
	DDRB = 0xFF;
	//DDRB |= 1<<PINB0 | 1<<PINB1 | 1<<PINB2 | 1<<PINB3 | 1<<PINB4 ;
	DDRC |= 1 <<PINC6;
	DDRC |= 1<<PINC7;
	DDRD &= ~(1<<PIND0) &~(1<<PIND1) &~(1<<PIND2) &~(1<<PIND3) &~(1<<PIND4) &~(1<<PIND6);
	DDRD |= 1<<PIND7;
	
	char data[5];
	lcd_init(LCD_BACKLIGHT_ON);			// Initialize LCD 
	lcd_clear();			// Clear LCD 
	lcd_goto_xy(2,0);		// column and row position 
	lcd_puts(" MULTIFUNCTIONAL");
	lcd_goto_xy(0,1);
	lcd_puts("    <<< BED >>>");
	_delay_ms(3000);
	lcd_clear();			
	lcd_goto_xy(2,0);		
	lcd_puts("       IT GROUP 01");
	_delay_ms(5000);
	lcd_clear();
	_delay_ms(1000);
	lcd_puts("Initiating...");
	_delay_ms(2000);
	lcd_clear();
	_delay_ms(1000);
	lcd_goto_xy(0,0);
	
	//int forward =1;
	
	
    while (1) 
    {
		lcd_puts("~ Temperature ~ ");
		
		Request();		/* send start pulse */
		Response();		/* receive response */
		I_RH=Receive_data();	/* store first eight bit in I_RH */
		D_RH=Receive_data();	/* store next eight bit in D_RH */
		I_Temp=Receive_data();	/* store next eight bit in I_Temp */
		D_Temp=Receive_data();	/* store next eight bit in D_Temp */
		CheckSum=Receive_data();/* store next eight bit in CheckSum */
		
		if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
		{
			lcd_goto_xy(0,0);
			lcd_puts("Error");
		}
		
		else
		{
			itoa(I_Temp,data,10);
			lcd_goto_xy(5,1);
			lcd_puts(data);
			lcd_puts(".");
			
			itoa(D_Temp,data,10);
			lcd_puts(data);  
			lcd_puts("C ");
			
			//DDRB |= (1<<PB1);
			if (I_Temp >= 0b00011110 )
			{
				
				lcd_goto_xy(0,0);
				lcd_puts("COOLING ON   ");
				//_delay_ms(1000);
				PORTB &= ~(1<<PB1);
				_delay_ms(1000);
				
			}
		
			else{
				PORTB |= (1<<PB1);             
				_delay_ms(1000);
			}
		
			//////////////////////////////////////////////////// Motion Detection with PIR Sensor ///////////////////////////////////////////////////////////////////
			
			if((PINA & (1<<PA0)) || (PIND & (1<<PD0)) )
			{
				PORTB &= ~(1<<PB0);
				_delay_ms(5000);
				PORTB |= (1<<PB0);

			}
			else
			{
				PORTB |= (1<<PB0);
			}
			
			int is_forward = 1;
			int delay;
			if(PIND & (1<<PIND3)){//switch                             ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////new///////////////////////////////
				
				if(PIND & (1 << PIND6)){//direction check
					lcd_init(LCD_BACKLIGHT_ON);
					lcd_clear();			// Clear LCD
					lcd_goto_xy(2,0);		// column and row position
					lcd_puts(" TABLE FORWARD");
					is_forward = 1;
					delay = 1;
				}
				else if(PIND & (1 << PIND4)){
					lcd_init(LCD_BACKLIGHT_ON);
					lcd_clear();			// Clear LCD
					lcd_goto_xy(2,0);		// column and row position
					lcd_puts(" TABLE BACKWARD");
					is_forward = 0;
					delay = 1;
				}
				
				while(1){
					if(is_forward){//forward
						PORTC &= ~(1<<PC6);		
						PORTC |= 1<<PC7;
					}else if(is_forward == 0){//backward
						PORTC |= (1<<PC6);
						PORTC &= ~(1<<PC7);
					}
					if(delay){
					_delay_ms(9000);
						delay = 0;
					}else
						_delay_ms(1000);
					
					if((PIND & (1 << PIND6)) || (PIND & (1 << PIND4))){
						PORTC |= (1<<PC6);
						PORTC |= (1<<PC7);
						break;
					}
				}
				if(is_forward){
					lcd_init(LCD_BACKLIGHT_ON);
					lcd_clear();			// Clear LCD
					lcd_goto_xy(2,0);		// column and row position
					lcd_puts(" TABLE LAMP ON");
					PORTB |= 1<<PB6;		// study lamp   PB6 MOTOR on
					PORTB &= ~(1<<PB5);		// study lamp   PB5 MOTOR off
					_delay_ms(22000);
					
					PORTB &= ~(1<<PB5);
					PORTB &= ~(1<<PB6);
					if(PIND & (1<<PIND1)) // if LDR=1
						PORTB |= 1<<PB7; // study lamp on
				}else{
					lcd_init(LCD_BACKLIGHT_ON);
					lcd_clear();			// Clear LCD
					lcd_goto_xy(2,0);		// column and row position
					lcd_puts(" TABLE LAMP OFF");
					PORTB &= ~(1<<PB7);
					PORTB &= ~(1<<PB6);		// study lamp   PB6 MOTOR on
					PORTB |= (1<<PB5);		// study lamp   PB5 MOTOR off
					_delay_ms(22000);
					PORTB &= ~(1<<PB5);
					PORTB &= ~(1<<PB6);
				}
				
			}
		
		}
	_delay_ms(1000);
	lcd_clear();
	_delay_ms(1000);
		
	}
}



