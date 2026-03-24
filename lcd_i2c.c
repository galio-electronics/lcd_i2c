// ============================================================================
// Driver I2C para LCD HD44780 via expansor (tipo PCF8574)
// Configurable desde main, sin tocar la libreria.
// ============================================================================

// -------------------------- Configuracion (override) ------------------------
// Desde el main puedes hacer, por ejemplo:
//
//   #define LCD_I2C_ADDR  0x4E
//   #define LCD_COLS      16
//   #define LCD_ROWS      2
//   #include "lcd_i2c.c"
//
// Y el driver tomara esos valores.

// Direccion I2C por defecto
#ifndef LCD_I2C_ADDR
   #define LCD_I2C_ADDR   0x4E
#endif

// Columnas/filas por defecto
#ifndef LCD_COLS
   #define LCD_COLS       16
#endif

#ifndef LCD_ROWS
   #define LCD_ROWS       2
#endif

// Tipo de LCD (para FUNCTION SET)
// 0=5x7, 1=5x10, 2=2 lineas
#ifndef LCD_TYPE
   #define LCD_TYPE 2
#endif

// Si quieres mapear a otro bus I2C, desde el main puedes redefinir:
//
//   #define LCD_I2C_START()   i2c_start(...)
//   #define LCD_I2C_WRITE(b)  i2c_write(...)
//   #define LCD_I2C_STOP()    i2c_stop(...)

#ifndef LCD_I2C_START
   #define LCD_I2C_START()      I2C_Start()
   #define LCD_I2C_WRITE(b)     I2C_Write(b)
   #define LCD_I2C_STOP()       I2C_Stop()
#endif

// -------------------------- Comandos y flags --------------------------------

#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00

#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4

#define LCD_CLEAR_             0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F
#define LCD_MOVE_CURSOR_LEFT   0x10
#define LCD_MOVE_CURSOR_RIGHT  0x14
#define LCD_TURN_ON            0x0C
#define LCD_TURN_OFF           0x08
#define LCD_SHIFT_LEFT         0x18
#define LCD_SHIFT_RIGHT        0x1E

// Flags ENTRY MODE
#define LCD_ENTRY_INC          0x02
#define LCD_ENTRY_SHIFT        0x01

// -------------------------- Estado interno ----------------------------------

// NOTA: solo manejamos un LCD global.
// Si despues quieres multi-instancia, se puede migrar a un struct.

static int1  _lcd_rs = 0;
static int8  _lcd_i2c_addr = LCD_I2C_ADDR;
static int8  _lcd_backlight = LCD_BACKLIGHT;

// -------------------------- Funciones internas ------------------------------
// Las defino antes para no necesitar prototipos.

static void _lcd_expander_write(int8 value)
{
   LCD_I2C_START();
   LCD_I2C_WRITE(_lcd_i2c_addr);
   LCD_I2C_WRITE(value | _lcd_backlight);
   LCD_I2C_STOP();
}

// n: dato en alto nibble alineado (por ejemplo 0x30, 0x20, etc.)
// Asumimos mapeo tipico: P7–P4 -> D7–D4, P2=EN, P1=RW, P0=RS
static void _lcd_write_nibble(int8 n)
{
   int8 data;

   // RS en bit 0
   data = (n & 0xF0) | (_lcd_rs ? 0x01 : 0x00);

   _lcd_expander_write(data);
   _lcd_expander_write(data | 0x04);    // EN = 1
   delay_us(1);
   _lcd_expander_write(data & ~0x04);   // EN = 0
   delay_us(50);
}

static void _lcd_write_byte(int8 value, int1 is_data)
{
   _lcd_rs = is_data;

   _lcd_write_nibble(value & 0xF0);           // alto nibble
   _lcd_write_nibble((value << 4) & 0xF0);    // bajo nibble
}

static void _lcd_command(int8 cmd)
{
   _lcd_write_byte(cmd, FALSE);
}

// -------------------------- API publica ------------------------------------
// (No hay prototipos separados, CCS esta mas contento asi)

void lcd_set_addr(int8 addr)
{
   _lcd_i2c_addr = addr;
}

void lcd_init(void)
{
   _lcd_i2c_addr   = LCD_I2C_ADDR;
   _lcd_backlight  = LCD_BACKLIGHT;
   _lcd_rs         = 0;

   // Secuencia de arranque tipica para 4-bit mode
   _lcd_expander_write(0);
   delay_ms(40);

   // 3 veces "function set" en 8-bit (alto nibble = 0x3)
   _lcd_write_nibble(0x30);
   delay_ms(5);
   _lcd_write_nibble(0x30);
   delay_ms(5);
   _lcd_write_nibble(0x30);
   delay_ms(5);

   // Cambiar a 4 bits (alto nibble = 0x2)
   _lcd_write_nibble(0x20);
   delay_ms(5);

   // Function set segun LCD_TYPE
   _lcd_command(0x20 | (LCD_TYPE << 2));
   delay_ms(5);

   // Display ON, cursor OFF
   _lcd_command(LCD_TURN_ON);
   delay_ms(5);

   // Clear
   _lcd_command(LCD_CLEAR_);
   delay_ms(5);

   // Entry mode: incrementar cursor, sin shift
   _lcd_command(LCD_ENTRY_MODE_SET | LCD_ENTRY_INC);
   delay_ms(5);

   // Cursor off (por si acaso)
   _lcd_command(LCD_CURSOR_OFF);
   delay_ms(5);
}

void lcd_clear(void)
{
   _lcd_command(LCD_CLEAR_);
   delay_ms(2);
}

void lcd_home(void)
{
   _lcd_command(LCD_RETURN_HOME);
   delay_ms(2);
}

void lcd_gotoxy(int8 col, int8 row)
{
   int8 addr;

   if(row == 1)
      addr = LCD_FIRST_ROW;
   else if(row == 2)
      addr = LCD_SECOND_ROW;
   else if(row == 3)
      addr = LCD_THIRD_ROW;
   else
      addr = LCD_FOURTH_ROW;

   if(col == 0)
      col = 1;

   _lcd_command(addr + (col - 1));
}

void lcd_putc(char c)
{
   _lcd_write_byte((int8)c, TRUE);
}

void lcd_print(ROM char *text)
{
   while(*text != '\0')
   {
      lcd_putc(*text);
      text++;
   }
}

void lcd_print_xy(int8 col, int8 row, ROM char *text)
{
   lcd_gotoxy(col, row);
   lcd_print(text);
}

void lcd_print_ram(char *text)
{
   while(*text != '\0')
   {
      lcd_putc(*text);
      text++;
   }
}

void lcd_print_xy_ram(int8 col, int8 row, char *text)
{
   lcd_gotoxy(col, row);
   lcd_print_ram(text);
}

void lcd_backlight_on(void)
{
   _lcd_backlight = LCD_BACKLIGHT;
   _lcd_expander_write(0);   // refresca estado
}

void lcd_backlight_off(void)
{
   _lcd_backlight = LCD_NOBACKLIGHT;
   _lcd_expander_write(0);   // refresca estado
}
