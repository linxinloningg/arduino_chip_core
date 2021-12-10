#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>

#include <time.h>                       
#include <sys/time.h>                  
#include <coredecls.h> 

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <CustomWiFiManager.h> 

#include <FS.h>

#include "SSD1306Wire.h"    //0.96寸用这个
#include "bilibilimage.h"
#include "seniverimage.h"


#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0
#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#define buttonPin D3            // 按钮引脚D3

ESP8266WebServer esp8266_server(80);    // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
int temp=27;

int humidity=30;

time_t now; //实例化时间
const String WDAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};  //星期
const String MONTH_NAMES[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};  //月份

const char* host_comment = "route.showapi.com";     // 将要连接的服务器地址  
const char* host_seniverse = "api.seniverse.com";     // 将要连接的服务器地址
const int httpPort = 80;                    // 将要连接的服务器端口

/*char *ssid="TP-LINK_B453";
char *password="12345678";*/

//英语短句：
String showapi_appid ="325145";
String showapi_timestamp ="20200812124057";
String showapi_sign ="4f6ad40dbc5d4a09968c2617c3c7e8c9";


// 心知天气HTTP请求所需信息
String reqUserKey = "S-U0LQDaePFLlMXuy";   // 私钥
String reqLocation = "Zhaoqing";            // 城市
String reqUnit = "c";                      // 摄氏/华氏

const int I2C_DISPLAY_ADDRESS = 0x3c;  //I2c地址默认
const int SDA_PIN = 5;  //引脚连接
const int SDC_PIN = 4;  //
SSD1306Wire     display(I2C_DISPLAY_ADDRESS, SDA_PIN, SDC_PIN);   // 0.96寸用这个


const int UPDATE_SENIVERSE_SEC = 5 * 60; // 5分钟更新一次天气
const int UPDATE_COMMENT_SEC = 15 * 60; // 15分钟更新一次comment
const int UPDATE_SCREEN_SEC = 1 * 60;
long timeSinceLastCOMMENTUpdate = 0;    //上次屏幕更新后的时间
long timeSinceLastCurrUpdate = 0;   //上次天气更新后的时间
long timeSinceLastWUpdate = 0;    //上次评论更新后的时间
int screen=1;
String comment;

String weather;
int weather_code;
String temperature;
String last_update;

String WiFi_SSID;
IPAddress WiFi_LOCALIP;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);          
  Serial.println("");
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  
  display.init();
  display.clear();
  display.display();

  display.flipScreenVertically(); //屏幕翻转
  display.setContrast(255); //屏幕亮度

  //wificonnect_direct();
  
  bibililiAnimation(20);
  
  wificonnect();
  fileflash();
  webserver();
  init_display(&display);
}

void loop() 
{
  
  if (millis() - timeSinceLastCurrUpdate > (1000L * UPDATE_SENIVERSE_SEC)) 
  {httpRequest_seniverse;timeSinceLastCurrUpdate = millis();}
  if (millis() - timeSinceLastCOMMENTUpdate > (1000L * UPDATE_COMMENT_SEC))
  {httpRequest_comment;timeSinceLastCOMMENTUpdate = millis();}
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_SCREEN_SEC))
  {screen++;timeSinceLastWUpdate = millis();}
  if(digitalRead(buttonPin)==LOW)
  {delay(100);if(digitalRead(buttonPin)==LOW)screen++;}
  if(screen>4)screen=1;
  switch(screen)
  {
    case 1:  drawpage_1();break;
    case 2:  drawpage_2();break;
    case 3:  drawpage_3();break;
    case 4:  drawpage_4();break;
  }
  esp8266_server.handleClient(); 
}
void httpRequest_comment(){
  WiFiClient client;
  
  String URL= "/1211-1?count=1&showapi_appid=" + showapi_appid + "&showapi_timestamp=" + showapi_timestamp + "&showapi_sign=" + showapi_sign;

  String httpRequest = String("GET ") + URL + " HTTP/1.1\r\n" + 
                              "Host: " + host_comment + "\r\n" + 
                              "Connection: close\r\n\r\n";

  Serial.println(""); 
  Serial.print("Connecting to "); Serial.print(host_comment);

  // 尝试连接服务器
  if (client.connect(host_comment, 80))
  {
    Serial.println(" Success!");
 
    // 向服务器发送http请求信息
    client.print(httpRequest);
    Serial.println("Sending request: ");
    Serial.println(httpRequest);  
 
    // 获取并显示服务器响应状态行 
    Serial.println("Successfully received!");
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response: ");
    Serial.println(status_response);

    
 
    // 使用find跳过HTTP响应头
    if (client.find("\r\n\r\n")) 
    {Serial.println("Found Header End. Start Parsing.");}

    // 利用ArduinoJson库解析
    parseInfo_comment(client); 
  } 
  else {Serial.println(" connection failed!");}   
  //断开客户端与服务器连接工作
  client.stop(); 
}
void httpRequest_seniverse(){
  WiFiClient client;

// 建立心知天气API当前天气请求资源地址
  String URL = "/v3/weather/now.json?key=" + reqUserKey + "&location=" + reqLocation +  "&language=en&unit=" +reqUnit;
 
  // 建立http请求信息
  String httpRequest = String("GET ") + URL + " HTTP/1.1\r\n" + 
                              "Host: " + host_seniverse + "\r\n" + 
                              "Connection: close\r\n\r\n";
  Serial.println(""); 
  Serial.print("Connecting to "); Serial.print(host_seniverse);
 
  // 尝试连接服务器
  if (client.connect(host_seniverse, 80))
  {
    Serial.println(" Success!");
 
    // 向服务器发送http请求信息
    client.print(httpRequest);
    Serial.println("Sending request: ");
    Serial.println(httpRequest);  
 
    // 获取并显示服务器响应状态行 
    Serial.println("Successfully received!");
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response: ");
    Serial.println(status_response);
 
    // 使用find跳过HTTP响应头
    if (client.find("\r\n\r\n")) 
    {Serial.println("Found Header End. Start Parsing.");}
    
    // 利用ArduinoJson库解析心知天气响应信息
    parseInfo_seniverse(client); 
  } 
  else {Serial.println(" connection failed!");}   
  //断开客户端与服务器连接工作
  client.stop(); 
}
void parseInfo_comment(WiFiClient client){
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 330;
  DynamicJsonDocument doc(capacity);
  
  deserializeJson(doc, client);
  
  const char* showapi_res_error = doc["showapi_res_error"]; // ""
  const char* showapi_res_id = doc["showapi_res_id"]; // "5f3163048d57ba656e7d3ad4"
  int showapi_res_code = doc["showapi_res_code"]; // 0
  
  JsonObject showapi_res_body = doc["showapi_res_body"];
  int showapi_res_body_ret_code = showapi_res_body["ret_code"]; // 0
  const char* showapi_res_body_ret_message = showapi_res_body["ret_message"]; // "Success"
  
  const char* showapi_res_body_data_0_english = showapi_res_body["data"][0]["english"]; // "In the past only can be used to memory, don't wallow in the shadow of it, otherwise,always can't see the road ahead."
  const char* showapi_res_body_data_0_chinese = showapi_res_body["data"][0]["chinese"]; // "过去只可以用来回忆，别沉迷在它的阴影中，否则永远看不清前面的路。"

  comment=showapi_res_body["data"][0]["english"].as<String>();
  Serial.println("comment:");
  Serial.println(comment);
}
// 利用ArduinoJson库解析心知天气响应信息
void parseInfo_seniverse(WiFiClient client){
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(capacity);
  
  deserializeJson(doc, client);
  
  JsonObject results_0 = doc["results"][0];
  
  JsonObject results_0_now = results_0["now"];
  const char* results_0_now_text = results_0_now["text"]; // "Sunny"
  const char* results_0_now_code = results_0_now["code"]; // "0"
  const char* results_0_now_temperature = results_0_now["temperature"]; // "32"
  
  const char* results_0_last_update = results_0["last_update"]; // "2020-06-02T14:40:00+08:00" 
 
  // 通过串口监视器显示以上信息
   weather = results_0_now["text"].as<String>(); 
   weather_code = results_0_now["code"].as<int>(); 
   temperature = results_0_now["temperature"].as<String>(); 
   last_update= results_0["last_update"].as<String>();   
}
void handlelogin()
{
  String httpbody="<html><head><meta charset=\"utf-8\"><title>error</title></head><body><div><p>你输入的信息有误，请重新登陆!</p></div><div><a href=\"/login.html\"><<重新登陆</a></div></body></html>";
  String username=esp8266_server.arg("name");
  String password=esp8266_server.arg("password");
  if(username=="linxinloningg"&&password=="12345678")
  {esp8266_server.sendHeader("Location","/index.html");esp8266_server.send(303);}
  else
  esp8266_server.send(200,"text/html",httpbody);
}

void readWiFiname()
 {
  esp8266_server.send(200, "text/plain", WiFi_SSID);
 }
 void readWiFiip()
 {
  esp8266_server.send(200, "text/plain", "192.168.0.2");
 }
 void readtemperture()
 {
  String Temperature=(String)temp;
  esp8266_server.send(200, "text/plain", Temperature);
 }
 /****************************************************************************************************/

void handleledcontrol()
{
  String LEDState = "OFF";
  String DATA = esp8266_server.arg("DATA"); 
 
 if(DATA == "1"){
  digitalWrite(LED_BUILTIN,LOW); //LED 点亮
  LEDState = "ON"; //反馈参数
 } else {
  digitalWrite(LED_BUILTIN,HIGH); //LED 熄灭
  LEDState = "OFF"; //反馈参数
 }
 
 esp8266_server.send(200, "text/plain", LEDState); //发送网页
 }
 
 
void temperaturecontrol()
{
  String DATA = esp8266_server.arg("DATA");
  if(DATA=="1")temp++;
  else temp--;
  String Temperature=String (temp);
  esp8266_server.send(200, "text/plain", Temperature);
}

void humiditycontrol()
{
  String DATA = esp8266_server.arg("DATA");
  if(DATA=="1")humidity++;
  else humidity--;
  String Humidity=String (humidity);
  esp8266_server.send(200, "text/plain", Humidity);
}

void curtainscontrol()
{
  String curtainState = "OFF";
  String DATA = esp8266_server.arg("DATA"); 
 
 if(DATA == "1"){
  digitalWrite(LED_BUILTIN,LOW); //LED 点亮
  curtainState = "ON"; //反馈参数
 } else {
  digitalWrite(LED_BUILTIN,HIGH); //LED 熄灭
  curtainState = "OFF"; //反馈参数
 }
  esp8266_server.send(200, "text/plain", curtainState);
}
/********************************************************************************************************/


// 处理用户浏览器的HTTP访问
void handleUserRequet() {         
     
  // 获取用户请求网址信息
  String webAddress = esp8266_server.uri();
  
  // 通过handleFileRead函数处处理用户访问
  bool fileReadOK = handleFileRead(webAddress);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}

bool handleFileRead(String path) {            //处理浏览器HTTP访问

  if (path.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    path = "/login.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(path);  // 获取文件类型
  
  if (SPIFFS.exists(path)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(path, "r");          // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
/*void wificonnect_direct() {  //WIFI密码连接，Web配网请注释
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_5);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_6);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_7);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_8);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_1);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_2);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_3);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_4);
      display.display();}
    
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
    Serial.println(""); 
    Serial.print("ESP8266 Connected to ");
    WiFi_SSID=WiFi.SSID();
    Serial.println(WiFi_SSID); // WiFi名称
    WiFi_LOCALIP=WiFi.localIP();
    Serial.print("IP address:\t");
    Serial.println(WiFi_LOCALIP);           // IP
    delay(500);
}*/
void wificonnect()
{
      display.clear();
      display.drawXbm(0, 0, 128, 64, bilibili); //显示哔哩哔哩
      display.display();
    // 建立WiFiManager对象
    WiFiManager wifiManager;

    //清除遗留wifi信息
    if (WiFi.status() != WL_CONNECTED) {wifiManager.resetSettings();}
    
    // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
    wifiManager.autoConnect("AutoConnectAP");
    
    // 如果您希望该WiFi添加密码，可以使用以下语句：
    // wifiManager.autoConnect("AutoConnectAP", "12345678");
    // 以上语句中的12345678是连接AutoConnectAP的密码

    wifiManager.setPageTitle("欢迎来到小林的WiFi配置页");  //设置页标题
    if (!wifiManager.autoConnect("AutoConnectAP")) {Serial.println("连接失败并超时");ESP.restart();}
      
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
    Serial.println(""); 
    Serial.print("ESP8266 Connected to ");
    WiFi_SSID=WiFi.SSID();
    Serial.println(WiFi_SSID); // WiFi名称
    WiFi_LOCALIP=WiFi.localIP();
    Serial.print("IP address:\t");
    Serial.println(WiFi_LOCALIP);           // IP
    delay(500);
}
void fileflash()
{
  if(SPIFFS.begin()){                       // 启动闪存文件系统
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }  
}
void webserver()
{
  esp8266_server.on("/login",handlelogin);

  esp8266_server.on("/readtemperature",readtemperture);
  esp8266_server.on("/readWiFiname",readWiFiname);
  esp8266_server.on("/readWiFiip",readWiFiip);
  
  esp8266_server.on("/setLED",handleledcontrol); 
  esp8266_server.on("/setTEMP",temperaturecontrol);
  esp8266_server.on("/setHUM",humiditycontrol);
  esp8266_server.on("/setCURTAIN",curtainscontrol);
  
  esp8266_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求

  esp8266_server.begin();                           // 启动网站服务
  Serial.println("HTTP server started");  
}
void bibililiAnimation(int count)
{
      for(int i=0;i<count;i++)
      {display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_5);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_6);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_7);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_8);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_1);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_2);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_3);
      display.display();
      delay(80);
      display.clear();
      display.drawXbm(34, 0, bili_Logo_width, bili_Logo_height, bili_Logo_4);
      display.display();}  
}
void drawProgress(OLEDDisplay *display, int percentage, String label) {    //绘制进度
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(2, 28, 124, 10, percentage);
  display->display();
}
void dispaly_font(OLEDDisplay *display, int x,int y,String temp)  /*ArialMT_Plain_10代表10个像素点一个字符，因此x，y只能逐10递增，每增10移动一个字符，可选的有ArialMT_Plain_16*/
{
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, y, temp);
  
}
void dispaly_medfont(OLEDDisplay *display, int x,int y,String temp)  /*ArialMT_Plain_10代表10个像素点一个字符，因此x，y只能逐10递增，每增10移动一个字符，可选的有ArialMT_Plain_16*/
{
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, y, temp);
  
}
void dispaly_bigfont(OLEDDisplay *display, int x,int y,String temp)  /*ArialMT_Plain_10代表10个像素点一个字符，因此x，y只能逐10递增，每增10移动一个字符，可选的有ArialMT_Plain_16*/
{
  display->setColor(WHITE);
  display->setFont(ArialMT_Plain_24);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, y, temp);
  
}
void drawDateTime(OLEDDisplay *display) {  //显示时间
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];

  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  String date = WDAY_NAMES[timeInfo->tm_wday];

  sprintf_P(buff, PSTR("%04d-%02d-%02d  %s"), timeInfo->tm_year + 1900, timeInfo->tm_mon + 1, timeInfo->tm_mday, WDAY_NAMES[timeInfo->tm_wday].c_str());
  display->drawString(64, 5, String(buff));
  display->setFont(ArialMT_Plain_24);

  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  display->drawString(64, 22, String(buff));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
}
void init_display(OLEDDisplay *display)
{
  drawProgress(display, 0, "Updating Time...");
  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com");
  drawProgress(display, 33, "Updating weather...");
  httpRequest_seniverse();
  drawProgress(display, 66, "Updating comment...");
  httpRequest_comment();
  drawProgress(display, 100, "Done..."); 
  delay(4000);
}
void drawHorline(OLEDDisplay *display,int Start_x,int Start_y,int Length)    
{
  display->drawHorizontalLine(Start_x,Start_y,Length);
  
}
void drawVerline(OLEDDisplay *display,int Start_x,int Start_y,int Length)    /*x不超过128*/
{
  display->drawVerticalLine(Start_x, Start_y, Length);
}
void drawpage_1(){
  display.clear();
  drawDateTime(&display);
  drawHorline(&display,0,62,30);
  drawHorline(&display,0,52,126);
  display.display();
}
void drawpage_2()
{
  display.clear();
  dispaly_font(&display,10,0,"WEATHER:"+weather);
  dispaly_font(&display,10,10,"CODE:");
  dispaly_font(&display,51,10,(String)weather_code);
  dispaly_font(&display,10,20,"TEMP:");
  dispaly_font(&display,51,20,temperature);
  dispaly_font(&display,71,20,"°c");
  dispaly_font(&display,10,30,"LAST:");
  dispaly_font(&display,10,40,last_update);
  drawHorline(&display,32,62,30);
  drawHorline(&display,0,52,126);
  display.display();
}
void drawpage_3()
{
  display.clear();
  if(weather_code<=3)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height, sunny);
  else if(weather_code>3&&weather_code<=8)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height, cloudy);
  else if(weather_code=9)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height, overcast);
  else if(weather_code=10)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height, shower);
  else if(weather_code>10&weather_code<=12)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height, thundershower);
  else if(weather_code=13)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,lightRain );
  else if(weather_code=14)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,moderateRain );
  else if(weather_code=15)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,heavyRain );
  else if(weather_code=16)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,storm );
  else if(weather_code=17)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,heavyStorm );
  else if(weather_code=18)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,severeStorm );
  else if(weather_code=19)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,iceRain );
  else if(weather_code>19)display.drawXbm(10, 0, seniverse_Logo_width, seniverse_Logo_height,unknown );
  dispaly_bigfont(&display,60,10,temperature);
  dispaly_bigfont(&display,90,10,"°C");
  drawVerline(&display,50,0,40);
  drawHorline(&display,64,62,30);
  drawHorline(&display,0,52,128);
  display.display();
}
void drawpage_4()
{
  int str_len = comment.length() + 1; 
  char char_array[str_len];
  comment.toCharArray(char_array, str_len);
  display.clear();
  for(int i=0;i<comment.length();i++)
  { 
    if(i<=24) dispaly_font(&display,i*5,0,(String)char_array[i]);
    else if(i>24&&i<=48)dispaly_font(&display,(i-24)*5,10,(String)char_array[i]);
    else if(i>48&&i<=72)dispaly_font(&display,(i-48)*5,20,(String)char_array[i]);
    else if(i>72&&i<=96)dispaly_font(&display,(i-72)*5,30,(String)char_array[i]); 
    else if(i>96&&i<=120)dispaly_font(&display,(i-96)*5,40,(String)char_array[i]);
  }
  drawHorline(&display,96,62,30);
  drawHorline(&display,0,52,128);
  display.display();
}
