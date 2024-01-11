#include "lcd_init.h"

#define LCD_RES_Clr()  HAL_GPIO_WritePin(SPI4_RES_GPIO_Port, SPI4_RES_Pin, GPIO_PIN_RESET)//RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(SPI4_RES_GPIO_Port, SPI4_RES_Pin, GPIO_PIN_SET)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(SPI4_DC_GPIO_Port, SPI4_DC_Pin, GPIO_PIN_RESET)//DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(SPI4_DC_GPIO_Port, SPI4_DC_Pin, GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET)//CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(SPI4_BLK_GPIO_Port, SPI4_BLK_Pin, GPIO_PIN_RESET)//BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(SPI4_BLK_GPIO_Port, SPI4_BLK_Pin, GPIO_PIN_SET)
/******************************************************************************
      函数说明：LCD串行数据写入函数
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(uint8_t dat) 
{	
	HAL_StatusTypeDef ret;
	LCD_CS_Clr();
	ret = HAL_SPI_Transmit(&hspi4, &dat, 1, 10);
  LCD_CS_Set();	
}

void LCD_Writ_Bus_NOCS(uint8_t dat) 
{	
	HAL_StatusTypeDef ret;
	ret = HAL_SPI_Transmit(&hspi4, &dat, 1, 10);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
	uint8_t tempout[2];
	memset(&(tempout[0]), dat>>8, 1);
	memset(&(tempout[1]), dat, 1);
	HAL_StatusTypeDef ret;
	LCD_CS_Clr();
	ret = HAL_SPI_Transmit(&hspi4, tempout, 2, 10);
  LCD_CS_Set();	
}

uint8_t* tempout;
void LCD_WR_DATA_NOCS(uint16_t dat)
{
	//tempout = &dat;

	//HAL_StatusTypeDef ret;
	HAL_SPI_Transmit(&hspi4, &dat, 2, 10);
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(uint8_t dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
}

void SPI_LCD_Init(void)
{
	
	LCD_RES_Clr();//复位
	HAL_Delay(100);
	LCD_RES_Set();
	HAL_Delay(100);
	
	LCD_BLK_Set();//打开背光
  HAL_Delay(100);
		LCD_WR_REG(0x11);     
	HAL_Delay(120);               

	LCD_WR_REG(0x36);     
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);  

	LCD_WR_REG(0x3A);     
	LCD_WR_DATA8(0x05);   

	LCD_WR_REG(0xB2);     
	LCD_WR_DATA8(0x0C);   
	LCD_WR_DATA8(0x0C);   
	LCD_WR_DATA8(0x00);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x33);   

	LCD_WR_REG(0xB7);     
	LCD_WR_DATA8(0x71);   

	LCD_WR_REG(0xBB);     
	LCD_WR_DATA8(0x3B);   

	LCD_WR_REG(0xC0);     
	LCD_WR_DATA8(0x2C);   

	LCD_WR_REG(0xC2);     
	LCD_WR_DATA8(0x01);   

	LCD_WR_REG(0xC3);     
	LCD_WR_DATA8(0x13);   

	LCD_WR_REG(0xC4);     
	LCD_WR_DATA8(0x20);   

	LCD_WR_REG(0xC6);     
	LCD_WR_DATA8(0x0F);   

	LCD_WR_REG(0xD0);     
	LCD_WR_DATA8(0xA4);   
	LCD_WR_DATA8(0xA1);   

	LCD_WR_REG(0xD6);     
	LCD_WR_DATA8(0xA1);   

	LCD_WR_REG(0xE0);     
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x08);
	LCD_WR_DATA8(0x0A);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x07);
	LCD_WR_DATA8(0x21);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x39);
	LCD_WR_DATA8(0x39);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x3C);  

	LCD_WR_REG(0xE1);     
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x03);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x10);
	LCD_WR_DATA8(0x21);
	LCD_WR_DATA8(0x32);
	LCD_WR_DATA8(0x38);
	LCD_WR_DATA8(0x16);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x14);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x3D);  

	LCD_WR_REG(0x21);     
	LCD_WR_REG(0x29);     
	LCD_WR_REG(0x2C); 

} 





