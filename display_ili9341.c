/*
 * display_ili9341.c
 *
 * Created: May 2024
 * Author: Baris Dinc OH2UDS/TA7W
 * 
 * Functions for displaying text and graphics on ili9341 tft display
 */

// #include "SPI.h"
// #include "uSDR.h"  //Serialx
// #include "dsp.h"
#include "display_ili9341.h"
// #include "hmi.h"
#include "lib/ili9341/ili9341.h"

static screen_control_t sScreen =
{
    .mCursorX = 0,
    .mCursorY = 0,
    .mCursorType = 0,
    .mCanvasPaper = kBlack,
    .mCanvasInk = kWhite
};
ili9341_config_t ili9341_hw_config;

void PRN32(uint32_t *val)
{ 
    *val ^= *val << 13;
    *val ^= *val >> 17;
    *val ^= *val << 5;
}

void TestRandomLabels(screen_control_t *p_screen)
{
    assert_(p_screen);

    static uint32_t rnd_seed = 0xa5efddbd;

    PRN32(&rnd_seed);
    const int x = rnd_seed % 240;
    PRN32(&rnd_seed);
    const int y = rnd_seed % 312;

    TftPutTextLabel(p_screen, "OH2UDS TA7W", x, y, false);
    
    TftFullScreenSelectiveWrite(p_screen, 10000);
}

void TestBoxDraw(screen_control_t *p_screen)
{
    assert_(p_screen);

    static int x = 0, y = 0;
    if(!x && !y)
    {
        memset(p_screen->mpPixBuffer, 0xFF, sizeof(p_screen->mpPixBuffer));
        TftFullScreenWrite(p_screen);
    }
    TftClearRect8(p_screen, x, y);

    if(++x >= TEXT_WIDTH)
    {
        x = 0;
        if(++y >= TEXT_HEIGHT)
            y = 0;
    }
    TftFullScreenSelectiveWrite(p_screen, 16);
}


void display_ili9341_init()
{
    // gpio_MISO,      28
    // gpio_CS,        12
    // gpio_SCK,       10
    // gpio_MOSI,      11
    // gpio_RS,        5
    // gpio_DC         4

    
    sScreen.mpHWConfig = &ili9341_hw_config;
    ILI9341_Init(sScreen.mpHWConfig, spi1, 90 * MHz, 28, 12, 10, 11, 5, 4);
}


// char vet_char[50];


uint16_t font_last = 0;
uint16_t color_last = 0;
uint16_t color_back_last = 0;
uint16_t size_last = 0;
uint16_t x_char_last = 0;
uint16_t y_char_last = 0;


void tft_writexy_(uint16_t font, uint16_t color, uint16_t color_back, uint16_t x, uint16_t y, uint8_t *s)
{
  if(font != font_last)
  {
    if(font == 3)
    {
      // tft.setFreeFont(FONT3);                 // Select the font
      // tft.setTextSize(SIZE3);  
      x_char_last = 1;//X_CHAR3;
      y_char_last = 1;//Y_CHAR3;
      size_last = 8;//SIZE3;
    }
    else if (font == 2)
    {
      // tft.setFreeFont(FONT2);                 // Select the font
      // tft.setTextSize(SIZE2);      
      x_char_last = 1;//X_CHAR2;
      y_char_last = 1;//Y_CHAR2;
      size_last = 8;//SIZE2;
    }
    else
    {
      // tft.setFreeFont(FONT1);                 // Select the font
      // tft.setTextSize(SIZE1);  //size 1 = 10 pixels, size 2 =20 pixels, and so on
      x_char_last = 1;//X_CHAR1;
      y_char_last = 1;//Y_CHAR1;
      size_last = 8;//SIZE1;
    }
    font_last = font;
  }

    //TftClearScreenBuffer(&sScreen, color_back, color);
    //TftFullScreenWrite(&sScreen);

  if((color != color_last) || (color_back != color_back_last))
    {
//    tft.setTextColor(color, color_back);
//    sScreen.mCanvasPaper = color_back;
//    sScreen.mCanvasInk = color;
    color_last = color;
    color_back_last = color_back;
    }
    
//    TftClearScreenBuffer(&sScreen, kBlack, kRed);
//    TftFullScreenWrite(&sScreen);
//    TftPutTextLabel(&sScreen, (const char *)s, x * x_char_last * size_last, y * y_char_last * size_last, false);
    TftPutTextLabel(&sScreen, (const char *)s, x * x_char_last * size_last, y * y_char_last * size_last, false);
//    TftPutTextLabel(&sScreen, "TEST", 15, 15, false);
    TftFullScreenSelectiveWrite(&sScreen, 10000);
    

  //tft.drawString((const char *)s, x * x_char_last * size_last, y * y_char_last * size_last, 1);// Print the string name of the font
  //TftPutString(&sScreen, (const char *)s, x * x_char_last * size_last, y * y_char_last * size_last, color_back, color);                            
  //TftFullScreenWrite(&sScreen);
}




/* write text to display at line column plus a delta x and y */
void tft_writexy_plus(uint16_t font, uint16_t color, uint16_t color_back, uint16_t x, uint16_t x_plus, uint16_t y, uint16_t y_plus, uint8_t *s)
{
  if(font != font_last)
  {
    if(font == 3)
    {
      // tft.setFreeFont(FONT3);                 // Select the font
      // tft.setTextSize(SIZE3);  
      x_char_last = 1;//X_CHAR3;
      y_char_last = 1;//Y_CHAR3;
      size_last = 8;//SIZE3;
    }
    else if (font == 2)
    {
      // tft.setFreeFont(FONT2);                 // Select the font
      // tft.setTextSize(SIZE2);      
      x_char_last = 1;//X_CHAR2;
      y_char_last = 1;//Y_CHAR2;
      size_last = 8;//SIZE2;
    }
    else
    {
      // tft.setFreeFont(FONT1);                 // Select the font
      // tft.setTextSize(SIZE1);  //size 1 = 10 pixels, size 2 =20 pixels, and so on
      x_char_last = 1;//X_CHAR1;
      y_char_last = 1;//Y_CHAR1;
      size_last = 8;//SIZE1;
    }
    font_last = font;
  }

    //TftClearScreenBuffer(&sScreen, color_back, color);
    //TftFullScreenWrite(&sScreen);

  if((color != color_last) || (color_back != color_back_last))
    {
//    tft.setTextColor(color, color_back);
    color_last = color;
    color_back_last = color_back;    
    }
    
//    TftPutTextLabel(&sScreen, "TEST", 15, 15, false);
//   tft.drawString((const char *)s, (x * x_char_last * size_last)+x_plus, (y * y_char_last * size_last)+y_plus, 1);// Print the string name of the font
    TftPutTextLabel(&sScreen, (const char *)s, (x * x_char_last * size_last)+x_plus, (y * y_char_last * size_last)+y_plus, false);                            
    TftFullScreenSelectiveWrite(&sScreen, 10000);

}






// void tft_cursor(uint16_t font, uint16_t color, uint8_t x, uint8_t y)
// {
//     if(font == 3)
//     {
//       for(uint16_t i=1; i<((Y_CHAR3 * SIZE3)/8); i++)
//       {
//       tft.drawFastHLine (x * X_CHAR3 * SIZE3, ((y+1) * Y_CHAR3 * SIZE3) - i , ((X_CHAR3 * SIZE3)*9)/10, color);
//       }
//     }
//     else if (font == 2)
//     {
//       for(uint16_t i=1; i<((Y_CHAR2 * SIZE2)/8); i++)
//       {
//       tft.drawFastHLine (x * X_CHAR2 * SIZE2, ((y+1) * Y_CHAR2 * SIZE2) - i , X_CHAR2 * SIZE2, color);
//       }
//     }
//     else
//     {
//       for(uint16_t i=1; i<((Y_CHAR1 * SIZE1)/8); i++)
//       {
//       tft.drawFastHLine (x * X_CHAR1 * SIZE1, ((y+1) * Y_CHAR1 * SIZE1) - i , X_CHAR1 * SIZE1, color);
//       }
//     }
// }


// void tft_cursor_plus(uint16_t font, uint16_t color, uint8_t x, uint8_t x_plus, uint8_t y, uint8_t y_plus)
// {
//   static uint16_t x_old = 0;
  
//     if(font == 3)
//     {
//       for(uint16_t i=1; i<((Y_CHAR3 * SIZE3)/8); i++)
//       {
//         tft.drawFastHLine (x_old, (((y+1) * Y_CHAR3 * SIZE3) - i)+y_plus , ((X_CHAR3 * SIZE3)*9)/10, kBlack);
//         tft.drawFastHLine ((x * X_CHAR3 * SIZE3)+x_plus, (((y+1) * Y_CHAR3 * SIZE3) - i)+y_plus , ((X_CHAR3 * SIZE3)*9)/10, color);
//       }
//       x_old = (x * X_CHAR3 * SIZE3)+x_plus;
//     }
//     else if (font == 2)
//     {
//       for(uint16_t i=1; i<((Y_CHAR2 * SIZE2)/8); i++)
//       {
//         tft.drawFastHLine (x_old, (((y+1) * Y_CHAR2 * SIZE2) - i)+y_plus , X_CHAR2 * SIZE2, kBlack);
//         tft.drawFastHLine ((x * X_CHAR2 * SIZE2)+x_plus, (((y+1) * Y_CHAR2 * SIZE2) - i)+y_plus , X_CHAR2 * SIZE2, color);
//       }
//       x_old = (x * X_CHAR2 * SIZE2)+x_plus;
//     }
//     else
//     {
//       for(uint16_t i=1; i<((Y_CHAR1 * SIZE1)/8); i++)
//       {
//         tft.drawFastHLine (x_old, (((y+1) * Y_CHAR1 * SIZE1) - i)+y_plus , X_CHAR1 * SIZE1, kBlack);
//         tft.drawFastHLine ((x * X_CHAR1 * SIZE1)+x_plus, (((y+1) * Y_CHAR1 * SIZE1) - i)+y_plus , X_CHAR1 * SIZE1, color);
//       }
//       x_old = (x * X_CHAR1 * SIZE1)+x_plus;
//     }
// }



// /* used to allow calling from other modules, concentrate the use of tft variable locally */
// uint16_t tft_color565(uint16_t r, uint16_t g, uint16_t b)
// {
//   return tft.color565(r, g, b);
// }













// #define ABOVE_SCALE   12
// #define TRIANG_TOP   (ABOVE_SCALE + 6)
// #define TRIANG_WIDTH    8

// int16_t triang_x_min, triang_x_max;

// /*********************************************************
  
// *********************************************************/
// void display_fft_graf_top(void) 
// {
//   int16_t siz, j, x;  //i, y
//   uint32_t freq_graf_ini;
//   uint32_t freq_graf_fim;



//     //graph min freq
//     freq_graf_ini = (hmi_freq - ((FFT_NSAMP/2)*FRES) )/1000;
  
//     //graph max freq
//     freq_graf_fim = (hmi_freq + ((FFT_NSAMP/2)*FRES) )/1000;

   
//     //little triangle indicating the center freq
//     switch(dsp_getmode())  //{"USB","LSB","AM","CW"}
//     {
//       case 0:  //USB
//         triang_x_min = (display_WIDTH/2);
//         triang_x_max = (display_WIDTH/2)+TRIANG_WIDTH;
//         tft.fillTriangle(display_WIDTH/2, Y_MIN_DRAW - ABOVE_SCALE, display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)+TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, kYellow);
//         tft.fillTriangle((display_WIDTH/2)-1, Y_MIN_DRAW - ABOVE_SCALE, display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)-TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, kBlack);
//         break;
//       case 1:  //LSB
//         triang_x_min = (display_WIDTH/2)-TRIANG_WIDTH;
//         triang_x_max = (display_WIDTH/2);
//         tft.fillTriangle(display_WIDTH/2, Y_MIN_DRAW - ABOVE_SCALE, 
//                          display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)-TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, 
//                          kYellow);
//         tft.fillTriangle((display_WIDTH/2)+1, Y_MIN_DRAW - ABOVE_SCALE, 
//                           display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)+TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, 
//                           kBlack);
//         break;
//       case 2:  //AM
//         triang_x_min = (display_WIDTH/2)-TRIANG_WIDTH;
//         triang_x_max = (display_WIDTH/2)+TRIANG_WIDTH;
//         tft.fillTriangle((display_WIDTH/2)-TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)+TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, kYellow);
//         break;
//       case 3:  //CW = LSB
//         triang_x_min = (display_WIDTH/2)-(TRIANG_WIDTH*2/4);
//         triang_x_max = (display_WIDTH/2);   //-(TRIANG_WIDTH*1/4);
//         tft.fillTriangle(display_WIDTH/2, Y_MIN_DRAW - ABOVE_SCALE, 
//                          display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)-TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, 
//                          kYellow);
//         tft.fillTriangle((display_WIDTH/2)+1, Y_MIN_DRAW - ABOVE_SCALE, 
//                           display_WIDTH/2, Y_MIN_DRAW - TRIANG_TOP, (display_WIDTH/2)+TRIANG_WIDTH, Y_MIN_DRAW - ABOVE_SCALE, 
//                           kBlack);
//         break;
//     }

//     //erase old freqs on top of scale
//     tft.fillRect(0, Y_MIN_DRAW - TRIANG_TOP - Y_CHAR1, display_WIDTH, Y_CHAR1, kBlack);

//     //plot scale on top of waterfall
//     tft.drawFastHLine (0, Y_MIN_DRAW - 11, display_WIDTH, TFT_WHITE);
//     tft.fillRect(0, Y_MIN_DRAW - 10, display_WIDTH, 10, kBlack);
//     tft.fillRect(triang_x_min, Y_MIN_DRAW - 10, (triang_x_max - triang_x_min + 1), 11, tft.color565(25, 25, 25)); //shadow
    
//     x=0;
//     for(; freq_graf_ini < freq_graf_fim; freq_graf_ini+=1)
//     {
//       if((freq_graf_ini % 10) == 0)
//       {
//         tft.drawFastVLine (x, Y_MIN_DRAW - 11, 5, TFT_WHITE);
//       }
//       if((freq_graf_ini % 50) == 0)
//       {
//         tft.drawFastVLine (x-1, Y_MIN_DRAW - 11, 7, TFT_WHITE);
//         tft.drawFastVLine (x, Y_MIN_DRAW - 11, 7, TFT_WHITE);
//         tft.drawFastVLine (x+1, Y_MIN_DRAW - 11, 7, TFT_WHITE);
//       }
//       if((freq_graf_ini % 100) == 0)
//       {
//          tft.drawFastVLine (x-1, Y_MIN_DRAW - 11, 10, TFT_WHITE);
//          tft.drawFastVLine (x, Y_MIN_DRAW - 11, 10, TFT_WHITE);
//          tft.drawFastVLine (x+1, Y_MIN_DRAW - 11, 10, TFT_WHITE);
    
//          //write new freq values  on top of scale
//          sprintf(vet_char, "%lu", freq_graf_ini);
//          siz = strlen(vet_char);
//          if(x < (2*X_CHAR1))   //to much to left
//          {
//             tft_writexy_plus(1, kMagenta, kBlack,0,0,7,0,(uint8_t *)vet_char);  
//          }
//          else if((x+((siz-2)*X_CHAR1)) > display_WIDTH)  //to much to right
//          {
//             j = display_WIDTH - (siz*X_CHAR1);
//             tft_writexy_plus(1, kMagenta, kBlack,0,j,7,0,(uint8_t *)vet_char);  
//          }
//          else
//          {
//             j = x - (2*X_CHAR1);
//             tft_writexy_plus(1, kMagenta, kBlack,0,j,7,0,(uint8_t *)vet_char);  
//          }
//       }
//       x+=2;
//     }
  

// }







// uint8_t vet_graf_fft[GRAPH_NUM_LINES][FFT_NSAMP];    // [NL][NCOL]
// //uint16_t vet_graf_fft_pos = 0;
// /*********************************************************
  
// *********************************************************/
// void display_fft_graf(void) 
// {
//   uint16_t x, y;
//   uint16_t extra_color;



//   //erase waterfall area
//   //tft.fillRect(0, Y_MIN_DRAW, GRAPH_NUM_COLS, GRAPH_NUM_LINES, kBlack);

//   extra_color = tft.color565(25, 25, 25);  //light shadow on center freq

//   //plot waterfall
//   //vet_graf_fft[GRAPH_NUM_LINES][GRAPH_NUM_COLS]   [NL][NCOL]
//   for(y=0; y<GRAPH_NUM_LINES; y++)
//   {
//     //erase one waterfall line
//     tft.drawFastHLine (0, (GRAPH_NUM_LINES + Y_MIN_DRAW - y), GRAPH_NUM_COLS, kBlack);

//     for(x=0; x<GRAPH_NUM_COLS; x++)
//     {
//       //plot one waterfall line
//       if(vet_graf_fft[y][x] > 0)
//       {
//         if((x>=triang_x_min) && (x<=triang_x_max))  //tune shadow area
//         {
//           tft.drawPixel(x, (GRAPH_NUM_LINES + Y_MIN_DRAW - y), TFT_WHITE|extra_color); 
//         }
//         else
//         {
//           tft.drawPixel(x, (GRAPH_NUM_LINES + Y_MIN_DRAW - y), TFT_WHITE); 
//         }
//       }
//       else
//       {
//         if((x>=triang_x_min) && (x<=triang_x_max))  //tune shadow area
//         {
//           tft.drawPixel(x, (GRAPH_NUM_LINES + Y_MIN_DRAW - y), kBlack|extra_color); 
//         }
//       }
//     }
//   }


//   //move graph data one line up to open space for next FFT (= last line)
//   for(y=0; y<(GRAPH_NUM_LINES-1); y++)
//   {
//     for(x=0; x<GRAPH_NUM_COLS; x++)
//     {
//       vet_graf_fft[y][x] = vet_graf_fft[y+1][x];
//     }
//   }


 
// }






// void display_aud_graf_var(uint16_t aud_pos, uint16_t aud_var, uint16_t color)
// {  
//   int16_t x;
//   int16_t aud; 
  
//   for(x=0; x<AUD_GRAPH_NUM_COLS; x++)
//   {
//     aud = (aud_samp[aud_var][x+aud_pos]);
    
//     if(aud < AUD_GRAPH_MIN)  //check boundaries
//     {
//       tft.drawPixel((x + X_MIN_AUD_GRAPH), (Y_MIN_AUD_GRAPH + AUD_GRAPH_MAX - AUD_GRAPH_MIN), color);    //lower line    
//     }
//     else if(aud > AUD_GRAPH_MAX)  //check boundaries
//     {
//       tft.drawPixel((x + X_MIN_AUD_GRAPH), (Y_MIN_AUD_GRAPH + AUD_GRAPH_MAX - AUD_GRAPH_MAX), color);    //upper line 
//     }
//     else
//     {
//       tft.drawPixel((x + X_MIN_AUD_GRAPH), (Y_MIN_AUD_GRAPH + AUD_GRAPH_MAX - aud), color);        
//     }
//   }
  
// }




// void display_aud_graf(void)
// {
// uint16_t aud_pos;
// int16_t x;
// int16_t aud_samp_trigger;
  
//   //erase graphic area
//   //tft.fillRect(0, Y_MIN_DRAW - 10, display_WIDTH, 10, kBlack);
//   tft.fillRect(X_MIN_AUD_GRAPH, Y_MIN_AUD_GRAPH, AUD_GRAPH_NUM_COLS, (AUD_GRAPH_MAX - AUD_GRAPH_MIN + 1), kBlack);

//  // if(tx_enabled)
//   {
//  //   aud_samp_trigger = AUD_SAMP_MIC;  
//   }
// //  else
//   {
//     aud_samp_trigger = AUD_SAMP_I;
//   }

//   //find trigger point to start ploting
//   for(x=0; x<AUD_GRAPH_NUM_COLS; x++)
//   {
//      if((aud_samp[aud_samp_trigger][x+0] > 0) &&
//         (aud_samp[aud_samp_trigger][x+1] > 0) &&
//         (aud_samp[aud_samp_trigger][x+2] > 0) &&
//         (aud_samp[aud_samp_trigger][x+3] > 0) &&
//         (aud_samp[aud_samp_trigger][x+4] > 0))
//         {
//           break;
//         }
//   }
//   for(; x<AUD_GRAPH_NUM_COLS; x++)
//   {
//      if(aud_samp[aud_samp_trigger][x] < 0)
//         {
//           break;
//         }
//   }
//   aud_pos = x;

//   //plot each variable
//   display_aud_graf_var(aud_pos, AUD_SAMP_I, kGreen);
//   display_aud_graf_var(aud_pos, AUD_SAMP_Q, kCyan);
//   display_aud_graf_var(aud_pos, AUD_SAMP_MIC, kRed);
//   display_aud_graf_var(aud_pos, AUD_SAMP_A, kRed);
//   display_aud_graf_var(aud_pos, AUD_SAMP_PEAK, kYellow);
//   display_aud_graf_var(aud_pos, AUD_SAMP_GAIN, kMagenta);

// }


/*********************************************************
  Initial msgs on display  (after reset)
*********************************************************/
void display_ili9341_setup0(void) {
  char s[32];
  
  // tft.setRotation(ROTATION_SETUP);
 
  // tft.fillScreen(kBlack);

  sprintf(s, "B3N TRX");
  tft_writexy_plus(3, kYellow, kBlack, 2,10,1,0,(uint8_t *)s);

  //tft.fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color)
  //tft.drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color)
  // tft.drawRoundRect(35, 25, 250, 70, 15, kYellow);

  sprintf(s, "QO-100 SSB/AM/CW");
//  tft_writexy_plus(2, kYellow, kBlack, 0,0,3,10,(uint8_t *)s);
  tft_writexy_plus(1, kYellow, kBlack, 3,0,5,0,(uint8_t *)s);

  sprintf(s, "Sat Transceiver");  //name changed from uSDR Pico FFT
//  tft_writexy_plus(2, kYellow, kBlack, 0,10,4,10,(uint8_t *)s);
  tft_writexy_plus(1, kYellow, kBlack, 4,0,6,10,(uint8_t *)s);

  sprintf(s, "TA-NET");
  tft_writexy_plus(1, kBlue, kBlack, 3,0,9,0,(uint8_t *)s);
  sprintf(s, "10489.777 MHz");
  tft_writexy_plus(1, kBlue, kBlack, 2,0,10,0,(uint8_t *)s);
//#endif  
}




/*********************************************************
  
*********************************************************/
void display_ili9341_setup(void) {

//uint16_t x, y;
char s[32];
  
//  tft.init();
//  tft.setRotation(ROTATION_SETUP);

  // tft.fillScreen(kBlack);
  

  //tft.drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
  //tft.drawRect(X_MIN_AUD_GRAPH-1, Y_MIN_AUD_GRAPH-1, AUD_GRAPH_NUM_COLS+2, (AUD_GRAPH_MAX - AUD_GRAPH_MIN + 1)+2, TFT_WHITE);

  //plot scale on left of audio scope
  // tft.drawFastVLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH, (AUD_GRAPH_MAX - AUD_GRAPH_MIN), TFT_WHITE);
  
  // //tft.fillRect(0, Y_MIN_DRAW - 10, display_WIDTH, 10, kBlack);

  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH, 8, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+5, 5, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+15, 5, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+25, 8, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+35, 5, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+45, 5, TFT_WHITE);
  // tft.drawFastHLine (X_MIN_AUD_GRAPH - 11, Y_MIN_AUD_GRAPH+50, 8, TFT_WHITE);
 




  sprintf(s, "I");
  tft_writexy_plus(1, kGreen, kBlack, 8,6,1,0,(uint8_t *)s);
  sprintf(s, "Q");
  tft_writexy_plus(1, kCyan, kBlack, 8,6,2,0,(uint8_t *)s);
  sprintf(s, "A");
  tft_writexy_plus(1, kRed, kBlack, 8,6,3,0,(uint8_t *)s);
  sprintf(s, "MIC");
  tft_writexy_plus(1, kRed, kBlack, 10,4,1,0,(uint8_t *)s);
  sprintf(s, "PEAK");
  tft_writexy_plus(1, kYellow, kBlack, 10,4,2,0,(uint8_t *)s);
  sprintf(s, "GAIN");
  tft_writexy_plus(1, kMagenta, kBlack, 10,4,3,0,(uint8_t *)s);


/* 
  tft.setFreeFont(FONT1);                 // Select the font
  txt_size = 1;
  tft.setTextColor(kMagenta, kBlack);
  tft.setTextSize(txt_size);  //size 1 = 10 pixels, size 2 =20 pixels, and so on

  x = 0;
  y = 0;
  sprintf(vet_char, "%dx%d", display_WIDTH/(X_CHAR1 * txt_size) ,display_HEIGHT/(Y_CHAR1 * txt_size));
  //sprintf(vet_char, "%dx%d %dx%d",display_WIDTH ,display_HEIGHT
  //                , display_WIDTH/(X_CHAR1 * txt_size) ,display_HEIGHT/(Y_CHAR1 * txt_size));
  //tft.setCursor(0, 0);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size, 1);// Print the string name of the font

  x = 1;
  y = 1;
  sprintf(vet_char, "x=%d y=%d",x ,y);
  //tft.setCursor(x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size, 1);// Print the string name of the font
*/
/*  
  x = 0;
  y = 4;
  sprintf(vet_char, "x=%d y=%d",x ,y);
  //tft.setCursor(x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size, 1);// Print the string name of the font
*/

/*
  tft.setFreeFont(FONT2);                 // Select the font
  txt_size = 1;
  tft.setTextColor(kMagenta, kBlack);
  tft.setTextSize(txt_size);  //size 1 = 10 pixels, size 2 =20 pixels, and so on

  
  x = 0;
  y = 6;
  sprintf(vet_char, "%dx%d x=%d y=%d", display_WIDTH/(X_CHAR2 * txt_size), display_HEIGHT/(Y_CHAR2 * txt_size), x, y);
  //sprintf(vet_char, "x=%d y=%d",x ,y);
  //tft.setCursor(x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size, 1);// Print the string name of the font
  
  x = 5;
  y = 7;
  sprintf(vet_char, "x=%d  y=%d",x ,y);
  //tft.setCursor(x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size, 1);// Print the string name of the font
*/
/*
  x = 14;
  y = 10;
  sprintf(vet_char, "x=%d y=%d",x ,y);
  //tft.setCursor(x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size);
  //tft.println(vet_char);
  tft.drawString(vet_char, x * X_CHAR2 * txt_size, y * Y_CHAR2 * txt_size, 1);// Print the string name of the font
*/


/*

//  tft.startWrite();  //CS on for SPI

  tft.drawFastHLine (0, Y_MIN_DRAW, display_WIDTH, TFT_WHITE);
  for(x=0; x<=((display_WIDTH/(X_CHAR2 * SIZE2))*X_CHAR2); x+=X_CHAR2)
  {
    tft.drawFastVLine (x, Y_MIN_DRAW, 10, TFT_WHITE);
    //x+=X_CHAR2;
  }



  for(y=0; y<=((display_HEIGHT/(Y_CHAR2 * SIZE2))*Y_CHAR2); y+=Y_CHAR2)
  {
    tft.drawFastHLine (0, y, 10, TFT_WHITE);
    //y+=Y_CHAR2;
  }


  for(x=0; x<80; x++)
  {
    y = x/2;
    tft.drawPixel(x, y + Y_MIN_DRAW, kRed); 
    tft.drawPixel(x, y + Y_MIN_DRAW + 1, kRed); 
    tft.drawPixel(x, y + Y_MIN_DRAW + 2, kRed); 
  }

//  tft.endWrite();  //CS off for SPI


*/
   
} // main




/*********************************************************
  Initial msgs on display  (after reset)
*********************************************************/
void display_ili9341_countdown(bool show, uint16_t val) 
{
  char s[32];

  if(show == true)
  {
    // tft.drawRoundRect(260, 100, 50, 40, 10, TFT_ORANGE);
    sprintf(s, "%d", val);

    //tft.fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color)
    if(val < 10)
    {
       tft_writexy_plus(1, TFT_ORANGE, kBlack, 20,4,5,0,(uint8_t *)s);
      //tft.drawRoundRect(275, 100, 35, 40, 10, TFT_ORANGE);
    }
    else
    {
       tft_writexy_plus(1, TFT_ORANGE, kBlack, 19,4,5,0,(uint8_t *)s);
      //tft.drawRoundRect(260, 100, 50, 40, 10, TFT_ORANGE);
    }
  }
  else  //fill with black = erase = close the window
  {
    // tft.fillRoundRect(260, 100, 50, 40, 10, kBlack);
  }
}

void display_ili9341_test()
{
    sScreen.mCanvasPaper = kBlack;
    sScreen.mCanvasInk = kMagenta;
    TftClearScreenBuffer(&sScreen, kBlack, kRed);
    TftFullScreenWrite(&sScreen);

    TestRandomLabels(&sScreen);

char s[32];
    sprintf(s, "HEBE %d  ", 45);
    tft_writexy_(1, kRed, kBlack,10,10,(uint8_t *)s);  
    //tft_writexy_(1, TFT_BLUE, kBlack,10,10,(uint8_t *)s);  
    //TestRandomLabels(&sScreen);

}

void display_tft_loop(void) 
{



}