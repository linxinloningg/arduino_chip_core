#define SDA 6
#define SCL 7
#define DO_Light A0
#define AO_Light A1
#define DO_humidity A2
#define AO_humidity A3
#define DQ A4
int Light_level;
int Humidity_level;
int value;
unsigned char DS18B20_Check(void)      
{   
  unsigned char retry=0;
    
    while (analogRead(DQ)&&retry<200)
  {
    retry++;
    delayMicroseconds(1);
  };
  if(retry>=200)return 1;
  else retry=0;
    while (!analogRead(DQ)&&retry<240)
  {
    retry++;
    delayMicroseconds(1);
  };
  if(retry>=240)return 1;     
  return 0;
}
void DS18B20_Init(void)
{
  Rest_sign();
  while(DS18B20_Check())
  {
    Serial.println("DS18B2 is not ok!");
    return ;
  }
  Serial.println("DS18B20 is ok!");
}
void Rest_sign(void)
{
  digitalWrite(DQ,LOW);
  delayMicroseconds(750);
  digitalWrite(DQ,HIGH);
  delayMicroseconds(15);
}
unsigned char DS18B20_Read_Bit(void)
{
  unsigned char dat;
  digitalWrite(DQ,LOW);
  delayMicroseconds(2);
  digitalWrite(DQ,HIGH);
  delayMicroseconds(12);
  if(digitalRead(DQ))dat=1;
  else dat=0;
  delayMicroseconds(50);
  return dat;
}
unsigned char DS18B20_Read_Byte(void)
{
  unsigned char i,j,dat;
  dat=0;
  for(i=1;i<=8;i++)
  {
    j=DS18B20_Read_Bit();
    dat=(j<<7)|(dat>>1);
  }
  return dat;
}
void DS18B20_Write_Byte(unsigned char dat)
{
  unsigned char j;
  unsigned char testd;
  for(j=1;j<=8;j++)
  {
    testd=dat&0x01;
    dat=dat>>1;
    if(testd)
    {
      digitalWrite(DQ,LOW);
      delayMicroseconds(2);
      digitalWrite(DQ,HIGH);
      delayMicroseconds(60);
    }
    else
    {
      digitalWrite(DQ,LOW);
      delayMicroseconds(60);
      digitalWrite(DQ,HIGH);
      delayMicroseconds(2);
    }
   }
}
void DS18B20_Start(void)
{
  Rest_sign();
  DS18B20_Write_Byte(0xcc);
  DS18B20_Write_Byte(0x44);
}
void get_Temperature(void)
{
unsigned char temp;
unsigned char a,b;
unsigned char  temperature;
DS18B20_Start();
Rest_sign();
DS18B20_Write_Byte(0xcc);
DS18B20_Write_Byte(0xbe);
a=DS18B20_Read_Byte();  //255
b=DS18B20_Read_Byte();  //255
temperature=b;
temperature=a+(temperature<<8);
if((temperature&0xf800)==0xf800)
{
temperature=(~temperature)+1;
value=temperature*(-0.109);
}
else
{
value=temperature*0.109;
}
Serial.print("温度为：");
Serial.println(value);
}
void scan_DO(void)
{
  if((digitalRead(DO_Light)==1)||(digitalRead(DO_humidity)==1))
  {
    if(digitalRead(DO_Light)==1)Serial.println("未超过光照阈值");
    else Serial.println("超过光照阈值");
    if(digitalRead(DO_humidity)==1)Serial.println("未超过湿度阈值");
    else Serial.println("超过湿度阈值");
  }
}
void get_Light_level(void)
{
  Light_level=(1024-analogRead(AO_Light))*0.098;
  Serial.print("光照为：%");
  Serial.println(Light_level);
}
void get_Humidity_level(void)
{
  Humidity_level=analogRead(AO_humidity)*0.098;
  Serial.print("湿度为：%");
  Serial.println(Humidity_level);
}
unsigned char arry1[]={0x10,0x60,0x02,0x8C,0x00,0x00,0xFE,0x92,0x92,0x92,0x92,0x92,0xFE,0x00,0x00,0x00,
0x04,0x04,0x7E,0x01,0x40,0x7E,0x42,0x42,0x7E,0x42,0x7E,0x42,0x42,0x7E,0x40,0x00};
unsigned char arry2[]={0x00,0x00,0xFC,0x24,0x24,0x24,0xFC,0x25,0x26,0x24,0xFC,0x24,0x24,0x24,0x04,0x00,
0x40,0x30,0x8F,0x80,0x84,0x4C,0x55,0x25,0x25,0x25,0x55,0x4C,0x80,0x80,0x80,0x00}; 

unsigned char arry3[]={0x10,0x60,0x02,0x8C,0x00,0xFE,0x92,0x92,0x92,0x92,0x92,0x92,0xFE,0x00,0x00,0x00,
0x04,0x04,0x7E,0x01,0x44,0x48,0x50,0x7F,0x40,0x40,0x7F,0x50,0x48,0x44,0x40,0x00}; 
unsigned char arry4[]={0x00,0x00,0xFC,0x24,0x24,0x24,0xFC,0x25,0x26,0x24,0xFC,0x24,0x24,0x24,0x04,0x00,
0x40,0x30,0x8F,0x80,0x84,0x4C,0x55,0x25,0x25,0x25,0x55,0x4C,0x80,0x80,0x80,0x00}; 

unsigned char arry5[]={0x40,0x40,0x42,0x44,0x58,0xC0,0x40,0x7F,0x40,0xC0,0x50,0x48,0x46,0x40,0x40,0x00,
0x80,0x80,0x40,0x20,0x18,0x07,0x00,0x00,0x00,0x3F,0x40,0x40,0x40,0x40,0x78,0x00}; 
unsigned char arry6[]={0x00,0xFE,0x42,0x42,0x42,0xFE,0x00,0x42,0xA2,0x9E,0x82,0xA2,0xC2,0xBE,0x00,0x00,
0x80,0x6F,0x08,0x08,0x28,0xCF,0x00,0x00,0x2F,0xC8,0x08,0x08,0x28,0xCF,0x00,0x00};  

unsigned char arry7[]={0x82,0x9A,0x96,0x93,0xFA,0x52,0x52,0x80,0x7E,0x12,0x12,0x12,0xF1,0x11,0x10,0x00,
0x00,0x01,0x00,0xFE,0x93,0x92,0x93,0x92,0x92,0x92,0x92,0xFE,0x03,0x00,0x00,0x00}; 
unsigned char arry8[]={0x00,0xFC,0x84,0x84,0x84,0xFC,0x00,0x10,0x10,0x10,0x10,0x10,0xFF,0x10,0x10,0x00,
0x00,0x3F,0x10,0x10,0x10,0x3F,0x00,0x00,0x01,0x06,0x40,0x80,0x7F,0x00,0x00,0x00}; 
unsigned char arry9[]={0x10,0x60,0x02,0xCC,0x80,0x40,0x20,0x1E,0x02,0x02,0x02,0x3E,0x40,0x40,0x40,0x00,
0x04,0x04,0x7E,0x01,0x80,0x80,0x83,0x4D,0x51,0x21,0x51,0x49,0x87,0x80,0x80,0x00}; 
unsigned char arry10[]={0x04,0x04,0x04,0x84,0x6F,0x04,0x04,0x04,0xE4,0x04,0x8F,0x44,0x24,0x04,0x04,0x00,
0x04,0x02,0x01,0xFF,0x00,0x10,0x08,0x04,0x3F,0x41,0x40,0x40,0x40,0x40,0x78,0x00}; 
                                                                                                              
unsigned char bmp1[]={0x00,0x00,0x00,0x00,0x00,0x80,0xE0,0x30,0x18,0x08,0x08,0x1C,0x34,0x7F,0x7E,0x3C,
0x08,0x08,0x18,0x10,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3C,0xA3,0xC0,0x40,0x40,0x00,0x03,0x83,0x98,0x98,0x98,0x98,
0x8B,0x83,0x00,0x00,0xC0,0xE3,0x3C,0x00,0x80,0x80,0x80,0x00,0x30,0x70,0x30,0x00,
0x68,0x58,0x6C,0x26,0x33,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1E,0xFD,0x12,0x13,0x18,0x0F,0x06,0x04,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1F,0x78,0x40,0x40,0x40,0x50,0x50,0x30,0x30,0x70,0x70,
0x90,0xD0,0xC0,0xC0,0x40,0x78,0xDF,0x8E,0x9E,0x90,0xB8,0x50,0x40,0x00,0x00,0x00};
unsigned char bmp_hook[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x30,0x0C,0x03,0x00,
0x00,0x00,0x00,0x10,0x08,0x04,0x18,0x20,0x10,0x0C,0x03,0x00,0x00,0x00,0x00,0x00};

unsigned char character[][16]=
{
   {0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00,},//0
   {0x00,0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,},//1
   {0x00,0x70,0x08,0x08,0x08,0x08,0xF0,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00,},//2
   {0x00,0x30,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x18,0x20,0x21,0x21,0x22,0x1C,0x00,},//3
   {0x00,0x00,0x80,0x40,0x30,0xF8,0x00,0x00,0x00,0x06,0x05,0x24,0x24,0x3F,0x24,0x24,},//4
   {0x00,0xF8,0x88,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x20,0x20,0x20,0x11,0x0E,0x00,},//5
   {0x00,0xE0,0x10,0x88,0x88,0x90,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x20,0x1F,0x00,},//6
   {0x00,0x18,0x08,0x08,0x88,0x68,0x18,0x00,0x00,0x00,0x00,0x3E,0x01,0x00,0x00,0x00,},//7
   {0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00,},//8
   {0x00,0xF0,0x08,0x08,0x08,0x10,0xE0,0x00,0x00,0x01,0x12,0x22,0x22,0x11,0x0F,0x00,},//9
   {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00,},//:
   {0xF0,0x08,0xF0,0x80,0x60,0x18,0x00,0x00,0x00,0x31,0x0C,0x03,0x1E,0x21,0x1E,0x00,}//%
};
void I2cStart(void)
{
digitalWrite(SDA,HIGH);
delayMicroseconds(10);
digitalWrite(SCL,HIGH);
delayMicroseconds(10); //建立时间是SDA保持时间大于4.7us
digitalWrite(SDA,LOW);
delayMicroseconds(10); //保持时间大于4us
digitalWrite(SCL,LOW);
delayMicroseconds(10);
}
void I2cStop(void)
{
digitalWrite(SDA,LOW);
delayMicroseconds(10);
digitalWrite(SCL,HIGH);
delayMicroseconds(10); //建立时间大于4.7us
digitalWrite(SDA,HIGH);
delayMicroseconds(10);
}

unsigned char I2cSendByte(unsigned char dat)
{
unsigned char a=0; //最大255，一个机器周期为1us，最大延时255us。
for(a=0;a<8;a++) //要发送8位，从最高位开始
{
if((dat>>7)==1)
digitalWrite(SDA,HIGH);
else 
digitalWrite(SDA,LOW);//起始信号之后SCL=0，所以可以直接改变SDA信号
dat=dat<<1;
delayMicroseconds(10);
digitalWrite(SCL,HIGH);
delayMicroseconds(10);//建立时间>4.7us
digitalWrite(SCL,LOW);
delayMicroseconds(10);//时间大于4us
}
digitalWrite(SDA,HIGH);
delayMicroseconds(10);;
digitalWrite(SCL,HIGH);
while(digitalRead(SDA)) //等待应答，也就是等待从设备把SDA拉低
{
delay(2); //如果超过2000us没有应答发送失败，或者为非应答，表示接收结束
digitalWrite(SCL,LOW);
delayMicroseconds(10);
return 0;
}
digitalWrite(SCL,LOW);
delayMicroseconds(10);
return 1;
}

void oled_Write_cmd(unsigned char cmd)
{
I2cStart();
I2cSendByte(0x78); //发送写器件地址
I2cSendByte(0x00); //发送要写入内存地址
I2cSendByte(cmd); //写入命令
I2cStop();
}

void oled_Write_data(unsigned char dat)
{
I2cStart();
I2cSendByte(0x78);//发送写器件地址
I2cSendByte(0x40);//发送要写入内存地址
I2cSendByte(dat); //写入数据
I2cStop();
}

void oled_clear(void)
{ unsigned char i,j;
oled_Write_cmd(0x20);
oled_Write_cmd(0x02);
for(i=0;i<8;i++)
{
oled_Write_cmd(0xb0+i);
oled_Write_cmd(0x00);
oled_Write_cmd(0x10);
for(j=0;j<128;j++)
{
oled_Write_data(0x00);
}
}
}

void oled_initial(void)//oled初始化函数
{
delayMicroseconds(500);
oled_Write_cmd(0xae);//--turn off oled panel 关闭显示
oled_Write_cmd(0x00);//---set low column address设置起始列的低四位 0x0x
oled_Write_cmd(0x10);//---set high column address设置起始列的高四位0x1x
oled_Write_cmd(0x40);//--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
oled_Write_cmd(0x81);//--set contrast control register设置对比度
oled_Write_cmd(0xff); // Set SEG Output Current Brightness对比度为oxff
oled_Write_cmd(0xa1);//--Set SEG/Column Mapping 0xa0左右反置 0xa1正常
oled_Write_cmd(0xc8);//Set COM/Row Scan Direction 0xc0上下反置 0xc8正常
oled_Write_cmd(0xa6);//--set normal display
oled_Write_cmd(0xa8);//--set multiplex ratio(1 to 64)
oled_Write_cmd(0x3f);//--1/64 duty
oled_Write_cmd(0xd3);//-set display offset Shift Mapping RAM Counter (0x00~0x3F)
oled_Write_cmd(0x00);//-not offset
oled_Write_cmd(0xd5);//--set display clock divide ratio/oscillator frequency
oled_Write_cmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
oled_Write_cmd(0xd9);//--set pre-charge period
oled_Write_cmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
oled_Write_cmd(0xda);//--set com pins hardware configuration
oled_Write_cmd(0x12);
oled_Write_cmd(0xdb);//--set vcomh
oled_Write_cmd(0x40);//Set VCOM Deselect Level
oled_Write_cmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)设置地址模式
//水平寻址，垂直寻址，页寻址
oled_Write_cmd(0x02);// 地址模式为页寻址
oled_Write_cmd(0x8d);//--set Charge Pump enable/disable
oled_Write_cmd(0x14);//--set(0x10) disable
oled_Write_cmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
oled_Write_cmd(0xa6);// Disable Inverse Display On (0xa6/a7)
oled_Write_cmd(0xaf);//--turn on oled panel开启显示
delayMicroseconds(200);
oled_clear();//清屏
}

void oled_put_char_16x16(unsigned char x,unsigned char y,unsigned char t)
//设置显示坐标函数,t为0时，字符为8x16t为1时，字符为16x16
{
oled_Write_cmd(0x20);
oled_Write_cmd(0x00);//设置地址模式为水平选址
//set page
oled_Write_cmd(0x22);
oled_Write_cmd(y*2);
oled_Write_cmd(0x01+y*2);
//set colum
oled_Write_cmd(0x21);
oled_Write_cmd((0x08+0x08*t)*x);
oled_Write_cmd((0x08+0x08*t)*x+(0x07+0x08*t));
}
void oled_setpos(unsigned char x,unsigned char y)
{
  oled_Write_cmd(0xb0+y);
  oled_Write_cmd((((x+2)&0xf0)>>4)|0x10);
  oled_Write_cmd(((x+2)&0x0f));
}

void oled_showpictues(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
  unsigned int i=0;
  unsigned char x,y;
  //if(y1%8==0) y=y1/8;
  //else y=y1/8+1;
  y1/=8;
  for(y=y0;y<y1;y++)
  {
    oled_setpos(x0,y);
    for(x=x0;x<x1;x++)
      {oled_Write_data(BMP[i++]);}
  }
}
void oled_showchinese(unsigned char x,unsigned char y,unsigned char character[])
{
    oled_put_char_16x16(x,y,1);
    for(int j=0;j<32;j++)
    oled_Write_data(character[j]);
}
void oled_showchar(int horizontal,int vertical,int order)        
{
  int i;
  oled_put_char_16x16(horizontal,vertical,0);
  for(i=0;i<16;i++)
  {
  oled_Write_data(character[order][i]);
  }
}
void dispaly_interface(void)
{
  oled_showpictues(0,0,32,32,bmp1);
  oled_showchinese(2,0,arry1);
  oled_showchinese(3,0,arry2);
  oled_showchar(8,0,10);
  oled_showchinese(6,0,arry2);
  oled_showchinese(2,1,arry5);
  oled_showchinese(3,1,arry6);
  oled_showchar(8,1,10);
  oled_showchar(9,1,11);
  oled_showchinese(2,2,arry3);
  oled_showchinese(3,2,arry4);
  oled_showchar(8,2,10);
  oled_showchar(9,2,11);
  oled_showchinese(0,3,bmp_hook);
  oled_showchinese(2,3,arry7);
  oled_showchinese(3,3,arry8);
  oled_showchinese(4,3,arry9);
  oled_showchinese(5,3,arry10);
}
void dispaly_value(void)
{
  oled_showchar(9,0,value/10);
  oled_showchar(10,0,value%10);
  oled_showchar(10,1,Light_level/10);
  oled_showchar(11,1,Light_level%10);
  oled_showchar(10,2,Humidity_level/10);
  oled_showchar(11,2,Humidity_level%10);
}
void setup() {
  // put your setup code here, to run once:
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(DO_Light,INPUT);
  pinMode(AO_Light,INPUT);
  pinMode(DO_humidity,INPUT);
  pinMode(AO_humidity,INPUT);
  pinMode(DQ,INPUT);
  Serial.begin(9600);
  DS18B20_Init();
  oled_initial();
  dispaly_interface();
  
}

void loop() {
  // put your main code here, to run repeatedly:
 // scan_DO();
  get_Light_level();
  get_Humidity_level();
  get_Temperature();
  dispaly_value();
}
