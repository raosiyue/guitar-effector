
#include "delay.h"
#include "effect_chain.h"
#include "lcd.h"
#include "lcd_init.h"
#include "gif.h"
#include "pic.h"
#define NOSAVETIME 1

typedef enum UISTATE_e{
	UNKOWN,
	INTUNNER,
	NORMAL
}UISTATE;

void spi_lcd_handle(){
		static UISTATE statenow = UNKOWN;
		static uint8_t frameid = 0;
		static uint8_t boost_disp_sts = 0;
		static uint8_t od_disp_sts = 0;
		static uint8_t reverb_disp_sts = 0;
		static uint16_t line_endx = 0;
		static uint16_t line_endy = 0;
		static uint16_t line_startx = 0;
		static uint16_t line_starty = 0;
		static uint16_t centerx = 116;
		static uint16_t centery = 206;
		frameid++;
		if (frameid == 29){
			frameid = 1;
		}
		switch (statenow){
			case UNKOWN:
					if (function_mode.tunner_state){
						statenow = INTUNNER;
						LCD_ShowPicture(0,0,240,304,gImage_tunner);
					}else{
						statenow = NORMAL;
						LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
						boost_disp_sts = 0;
						od_disp_sts = 0;
						reverb_disp_sts = 0;
					}
					break;
			case INTUNNER:
					if (function_mode.tunner_state == 0){
						LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
						statenow = NORMAL;
						boost_disp_sts = 0;
						od_disp_sts = 0;
						reverb_disp_sts = 0;
					}else{
						if (function_mode.freq < 1){
								LCD_ShowString(110,190,"N",RED,BLACK,32,0);
								LCD_ShowIntNum(110,220,function_mode.std_freq,4,WHITE,BLACK,16);
								LCD_ShowIntNum(0,0,function_mode.freq,4,WHITE,BLACK,16);	

								LCD_DrawLine(line_startx,line_starty,line_endx,line_endy,BLACK);
								//clear last line
						}else if (function_mode.freq > 1){
							int getlen = 0;
							static uint16_t last_freq = 0;
							static uint16_t last_real_freq = 0;
							float diff_freq = 0;
							
							LCD_ShowChar(110,190,function_mode.string_name,WHITE,BLACK,32,0);
							LCD_ShowIntNum(110,220,function_mode.std_freq,4,WHITE,BLACK,16);
							LCD_ShowIntNum(0,0,function_mode.freq,4,WHITE,BLACK,16);
							//clear last line
							LCD_DrawLine(line_startx,line_starty,line_endx,line_endy,BLACK);
							diff_freq = 5 * (function_mode.std_freq - function_mode.freq);
							if (diff_freq > 40){
								diff_freq = 40;
							}
							if (diff_freq < -40){
								diff_freq = -40;
							}
							line_startx = centerx - 40 * sin((diff_freq) * 3.14 / 180);
							line_starty = centery - 40 * cos((diff_freq) * 3.14 / 180);
							line_endx = centerx - 140 * sin((diff_freq) * 3.14 / 180);
							line_endy = centery - 140 * cos((diff_freq) * 3.14 / 180);

							LCD_DrawLine(line_startx,line_starty,line_endx,line_endy,WHITE);
							//draw new line
						}
					}
					break;
				
			case NORMAL:
					if (function_mode.tunner_state){
						statenow = INTUNNER;
						LCD_ShowPicture(0,0,240,304,gImage_tunner);
						boost_disp_sts = 0;
						od_disp_sts = 0;
						reverb_disp_sts = 0;
						break;
					}
					LCD_ShowPictureBinary(40,150,152,152,taikongren[frameid]);
					if (function_mode.boost_state){
						if (boost_disp_sts == 0){
							boost_disp_sts = 1;
							LCD_ShowPicture(0,0,72,140,gImage_boost);
						}
					}else{
						
						if (boost_disp_sts == 1){
								LCD_Fill(0,0,72,140,BLACK);
								boost_disp_sts = 0;
						}
					}
					if (function_mode.od_state){
						if (od_disp_sts == 0){
							LCD_ShowPicture(80,0,75,140,gImage_sd1);
							od_disp_sts  = 1;
						}
					}else{
						if (od_disp_sts == 1){
							LCD_Fill(80,0,80 + 75,140,BLACK);
							od_disp_sts = 0;
						}
					}
					if (function_mode.reverb_state){
						//LCD_ShowString(0,40,"Reverb:",RED,BLACK,16,0);
						if (reverb_disp_sts == 0){
							LCD_ShowPicture(160,0,72,140,gImage_reverb);
							reverb_disp_sts = 1;
						}
					}else{
						//LCD_ShowString(0,40,"       ",RED,BLACK,16,0);
						if (reverb_disp_sts == 1){
							LCD_Fill(160,0,160 + 72,140,BLACK);
							reverb_disp_sts = 0;
						}
					}
					break;
		}
}

