#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>  

char *ssid="TP-LINK_B453";
char *password="12345678";

int temperature=27;

int humidity=30;

ESP8266WebServer esp8266_server(80);    // 建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);          // 启动串口通讯
  Serial.println("");
  WiFi.begin(ssid,password);
  Serial.println("Connecting ...");                            // 则尝试使用此处存储的密码进行连接。
  
  int i = 0;  
  while (WiFi.status() != WL_CONNECTED) { // 尝试进行wifi连接。
    delay(1000);
    Serial.print(i++); Serial.print(' ');
  }
  
  // WiFi连接成功后将通过串口监视器输出连接成功信息 
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP

  if(SPIFFS.begin()){                       // 启动闪存文件系统
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
  
  esp8266_server.on("/login",handlelogin);

  esp8266_server.on("/readtemperature",readtemperture);
  
  esp8266_server.on("/setLED",handleledcontrol); 
  esp8266_server.on("/setTEMP",temperaturecontrol);
  esp8266_server.on("/setHUM",humiditycontrol);
  esp8266_server.on("/setCURTAIN",curtainscontrol);
  
  esp8266_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求

  esp8266_server.begin();                           // 启动网站服务
  Serial.println("HTTP server started");
}

void loop(void) {
  esp8266_server.handleClient();                    // 处理用户请求
}

/******************************************************************************************************/
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

void readtemperture()
 {
  String Temperature=String (temperature);
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
  if(DATA=="1")temperature++;
  else temperature--;
  String Temperature=String (temperature);
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
