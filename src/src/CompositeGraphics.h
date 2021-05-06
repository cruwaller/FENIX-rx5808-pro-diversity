#pragma once
#include "Font.h"
#include "TriangleTree.h"

#define NUM_OSD_BUFF 2


class CompositeGraphics
{
  public:
  const int xres;
  const int yres;
  char **backbuffer;
#if !NUM_OSD_BUFF
  char **frame;
#else
  char **osdBuffers[NUM_OSD_BUFF];
#endif
  int cursorX, cursorY, cursorBaseX;
  int frontColor, backColor;
  Font<CompositeGraphics> *font;

  TriangleTree<CompositeGraphics> *triangleBuffer;
  TriangleTree<CompositeGraphics> *triangleRoot;
  int trinagleBufferSize;
  int triangleCount;

  CompositeGraphics(int w, int h, int initialTrinagleBufferSize = 0)
    : xres(w), yres(h)
  {
    font = 0;
    cursorX = cursorY = cursorBaseX = 0;
    trinagleBufferSize = initialTrinagleBufferSize;
    triangleCount = 0;
    frontColor = 50;
    backColor = -1;
  }

  inline void setTextColor(int front, int back = -1)
  {
    //-1 = transparent back;
    frontColor = front;
    backColor = back;
  }

  void init()
  {
#if !NUM_OSD_BUFF
    frame = (char**)malloc(yres * sizeof(char*));
    backbuffer = (char**)malloc(yres * sizeof(char*));
    for(int y = 0; y < yres; y++)
    {
      frame[y] = (char*)malloc(xres);
      backbuffer[y] = (char*)malloc(xres);
    }
#else
    for (int x = 0; x < NUM_OSD_BUFF; x++) {
      osdBuffers[x] = (char**)malloc(yres * sizeof(char*));
      for(int y = 0; y < yres; y++)
      {
        osdBuffers[x][y] = (char*)malloc(xres);
      }
    }
    backbuffer = osdBuffers[0];
#endif

    triangleBuffer = (TriangleTree<CompositeGraphics>*)malloc(sizeof(TriangleTree<CompositeGraphics>) * trinagleBufferSize);
  }

  void deinit()
  {
    if (triangleBuffer) {
      free(triangleBuffer);
      triangleBuffer = NULL;
    }
#if !NUM_OSD_BUFF
    for(int y = 0; y < yres; y++)
    {
      if (backbuffer[y]) {
        free(backbuffer[y]);
        backbuffer[y] = NULL;
      }
      if (frame[y]) {
        free(frame[y]);
        frame[y] = NULL;
      }
    }
    if (backbuffer) {
      free(backbuffer);
      backbuffer = NULL;
    }
    if (frame) {
      free(frame);
      frame = NULL;
    }
#else
    for (int x = 0; x < NUM_OSD_BUFF; x++) {
      for(int y = 0; y < yres; y++) {
        if (osdBuffers[x][y]) {
          free(osdBuffers[x][y]);
          osdBuffers[x][y] = NULL;
        }
       }
      free(osdBuffers[x]);
      osdBuffers[x] = NULL;
    }
    backbuffer = NULL;
#endif
  }

#if NUM_OSD_BUFF
  void get_frames(QueueHandle_t queue_free)
  {
    for (int x = 0; x < NUM_OSD_BUFF; x++)
    {
      xQueueSend(queue_free, &osdBuffers[x], portMAX_DELAY);
    }
  }

  inline void set_backbuff(char** buff)
  {
    backbuffer = buff;
  }
  inline char** get_backbuff(void)
  {
    return backbuffer;
  }
#endif

  inline void setFont(Font<CompositeGraphics> &font)
  {
    this->font = &font;
  }

  inline void setCursor(int x, int y)
  {
    cursorX = cursorBaseX = x;
    cursorY = y;
  }

  void IRAM_ATTR print(const char *str, bool invert=false) // drawCharLarge(Graphics &g, int x, int y, char ch, int frontColor, int backColor, int xMultiplier, int yMultiplier)
  {
    if(!font) return;
    while(*str)
    {
      if ((*str >= 32) && (*str < 128)) {
        if (invert)
          font->drawChar(*this, cursorX, cursorY, *str, backColor, frontColor);
        else
          font->drawChar(*this, cursorX, cursorY, *str, frontColor, backColor);
      }
      cursorX += font->xres;
      if(cursorX + font->xres > xres || *str == '\n')
      {
        cursorX = cursorBaseX;
        cursorY += font->yres;
      }
      str++;
    }
  }

  void IRAM_ATTR printLarge(const char *str, int xMultiplier, int yMultiplier)
  {
    if(!font) return;
    while(*str)
    {
      if(*str >= 32 && *str < 128)
        font->drawCharLarge(*this, cursorX, cursorY, *str, frontColor, backColor, xMultiplier, yMultiplier);
      cursorX += font->xres*xMultiplier;
      if(cursorX + font->xres > xres || *str == '\n')
      {
        cursorX = cursorBaseX;
        cursorY += font->yres*yMultiplier;
      }
      str++;
    }
  }

  void IRAM_ATTR print(int number, int base = 10, int minCharacters = 1, bool invert=false)
  {
    bool sign = number < 0;
    if(sign) number = -number;
    const char baseChars[] = "0123456789ABCDEF";
    char temp[33];
    temp[32] = 0;
    int i = 31;
    do
    {
      temp[i--] = baseChars[number % base];
      number /= base;
    }while(number > 0);
    if(sign)
      temp[i--] = '-';
    for(;i > 31 - minCharacters; i--)
      temp[i] = ' ';
    print(&temp[i + 1], invert);
  }

  void IRAM_ATTR printLarge(int number, int xMultiplier, int yMultiplier, int base = 10, int minCharacters = 1)
  {
    bool sign = number < 0;
    if(sign) number = -number;
    const char baseChars[] = "0123456789ABCDEF";
    char temp[33];
    temp[32] = 0;
    int i = 31;
    do
    {
      temp[i--] = baseChars[number % base];
      number /= base;
    }while(number > 0);
    if(sign)
      temp[i--] = '-';
    for(;i > 31 - minCharacters; i--)
      temp[i] = ' ';
    printLarge(&temp[i + 1], xMultiplier, yMultiplier);
  }

  inline void begin(int clear = -1)
  {
    if (clear > -1) {
      for(int y = 0; y < yres; y++)
        //memset(backbuffer[y], clear, xres);
        for(int x = 0; x < xres; x++)
          backbuffer[y][x] = clear;
    }
    triangleCount = 0;
    triangleRoot = 0;
  }

  inline void dotFast(int x, int y, char color)
  {
    backbuffer[y][x] = color;
  }

  inline void dot(int x, int y, char color)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      backbuffer[y][x] = color;
  }

  inline void dotAdd(int x, int y, char color)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      backbuffer[y][x] = min(54, color + backbuffer[y][x]);
  }

  inline char get(int x, int y)
  {
    if((unsigned int)x < xres && (unsigned int)y < yres)
      return backbuffer[y][x];
    return 0;
  }

  void IRAM_ATTR xLine(int x0, int x1, int y, char color)
  {
    if(x0 > x1)
    {
      int xb = x0;
      x0 = x1;
      x1 = xb;
    }
    if(x0 < 0) x0 = 0;
    if(x1 > xres) x1 = xres;
    for(int x = x0; x < x1; x++)
      dotFast(x, y, color);
  }

  void IRAM_ATTR enqueueTriangle(short *v0, short *v1, short *v2, char color)
  {
    if(triangleCount >= trinagleBufferSize) return;
    TriangleTree<CompositeGraphics> &t = triangleBuffer[triangleCount++];
    t.set(v0, v1, v2, color);
    if(triangleRoot)
      triangleRoot->add(&triangleRoot, t);
    else
      triangleRoot = &t;
  }

  void IRAM_ATTR triangle(short *v0, short *v1, short *v2, char color)
  {
    short *v[3] = {v0, v1, v2};
    if(v[1][1] < v[0][1])
    {
      short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    if(v[2][1] < v[1][1])
    {
      short *vb = v[1]; v[1] = v[2]; v[2] = vb;
    }
    if(v[1][1] < v[0][1])
    {
      short *vb = v[0]; v[0] = v[1]; v[1] = vb;
    }
    int y = v[0][1];
    int xac = v[0][0] << 16;
    int xab = v[0][0] << 16;
    int xbc = v[1][0] << 16;
    int xaci = 0;
    int xabi = 0;
    int xbci = 0;
    if(v[1][1] != v[0][1])
      xabi = ((v[1][0] - v[0][0]) << 16) / (v[1][1] - v[0][1]);
    if(v[2][1] != v[0][1])
      xaci = ((v[2][0] - v[0][0]) << 16) / (v[2][1] - v[0][1]);
    if(v[2][1] != v[1][1])
      xbci = ((v[2][0] - v[1][0]) << 16) / (v[2][1] - v[1][1]);

    for(; y < v[1][1] && y < yres; y++)
    {
      if(y >= 0)
        xLine(xab >> 16, xac >> 16, y, color);
      xab += xabi;
      xac += xaci;
    }
    for(; y < v[2][1] && y < yres; y++)
    {
      if(y >= 0)
        xLine(xbc >> 16, xac >> 16, y, color);
      xbc += xbci;
      xac += xaci;
    }
  }

  void IRAM_ATTR line(int x1, int y1, int x2, int y2, char color)
  {
    int x, y, xe, ye;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int dx1 = labs(dx);
    int dy1 = labs(dy);
    int px = 2 * dy1 - dx1;
    int py = 2 * dx1 - dy1;
    if(dy1 <= dx1)
    {
      if(dx >= 0)
      {
        x = x1;
        y = y1;
        xe = x2;
      }
      else
      {
        x = x2;
        y = y2;
        xe = x1;
      }
      dot(x, y, color);
      for(int i = 0; x < xe; i++)
      {
        x = x + 1;
        if(px < 0)
        {
          px = px + 2 * dy1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            y = y + 1;
          }
          else
          {
            y = y - 1;
          }
          px = px + 2 *(dy1 - dx1);
        }
        dot(x, y, color);
      }
    }
    else
    {
      if(dy >= 0)
      {
        x = x1;
        y = y1;
        ye = y2;
      }
      else
      {
        x = x2;
        y = y2;
        ye = y1;
      }
      dot(x, y, color);
      for(int i = 0; y < ye; i++)
      {
        y = y + 1;
        if(py <= 0)
        {
          py = py + 2 * dx1;
        }
        else
        {
          if((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
          {
            x = x + 1;
          }
          else
          {
            x = x - 1;
          }
          py = py + 2 * (dx1 - dy1);
        }
        dot(x, y, color);
      }
    }
  }

  inline void flush()
  {
    if(triangleRoot)
      triangleRoot->draw(*this);
  }

#if !NUM_OSD_BUFF
  inline char** end()
  {
    /* Swap the buffer and return the ready frame */
    char **b = backbuffer;
    backbuffer = frame;
    frame = b;
    return b;
  }

  inline char** get_frame()
  {
    return frame;
  }
#endif

  void IRAM_ATTR fillRect(int x, int y, int w, int h, int color)
  {
    if(x < 0)
    {
      w += x;
      x = 0;
    }
    if(y < 0)
    {
      h += y;
      y = 0;
    }
    if(x + w > xres)
      w = xres - x;
    if(y + h > yres)
      h = yres - y;
    for(int j = y; j < y + h; j++)
      for(int i = x; i < x + w; i++)
        dotFast(i, j, color);
  }

  inline void rect(int x, int y, int w, int h, int color)
  {
    fillRect(x, y, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x + w - 1, y, 1, h, color);
  }
};

