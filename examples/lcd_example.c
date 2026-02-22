/* 
 * File:   lcd_example
 * Author: Arturo Gasca
 *
 * Created on 11 de Diciembre de 2025, 10:46 AM
 */



#define PROTOLINK_DEFAULT     true
#include <protolink/v2.h> //<-- https://github.com/galio-electronics/protolink
#include <bootloader.h>
#include <stdio.h>
#include <stdlib.h>
#define FW_VERSION "1.0.3"

// Configuración I2C del micro
#use i2c(Master,I2C1,FAST=400000)

// Configuración del LCD DESDE EL MAIN
#define LCD_I2C_ADDR   0x7E   // Ajustar si tu módulo es 0x27 o similar
#define LCD_COLS       16
#define LCD_ROWS       2
#include "../lcd_i2c.c"




void main(void) {
    delay_ms(100);
    protolink_io_init();
    protolink_timer0_init();
    //protolink_interrupts_init();
    output_low(LED1);
    output_low(LED2);
    lcd_init(); //usa defines del main
    
    lcd_clear();
    lcd_print_xy(1,1,"Galio Electronics");
    lcd_print_xy(1,2,"LCD I2C OK");
    
   
      while(TRUE)
    {
      //Demo sencilla: parpadea backlight
          lcd_backlight_off();
          delay_ms(500);
          lcd_backlight_on();
          delay_ms(500);
          protolink_debug_msg("hello world\r\n");
    }

    
}