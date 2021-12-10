
#include <Servo.h>                //使用servo库
#define Trig A1
#define Echo A2
#define Hang 1
#define Free 0
Servo base, fArm, rArm, claw ;    //创建4个servo对象
 
//存储电机极限值(const指定该数值为常量,常量数值在程序运行中不能改变)
const int baseMin = 0;
const int baseMax = 180;
const int rArmMin = 45;
const int rArmMax = 180;
const int fArmMin = 35;
const int fArmMax = 180;
const int clawMin = 60;
const int clawMax = 180;
 
int DSD = 15; //Default Servo Delay (默认电机运动延迟时间)
              //此变量用于控制电机运行速度.增大此变量数值将
              //降低电机运行速度从而控制机械臂动作速度。
int DES_MAX=5;
int DES_MIN=3;
int Flag;
long duration, cm;
 
void setup(){
  base.attach(3);     // base 伺服舵机连接引脚11 舵机代号'b'
  delay(200);          // 稳定性等待
  rArm.attach(9);     // rArm 伺服舵机连接引脚10 舵机代号'r'
  delay(200);          // 稳定性等待
  fArm.attach(6);      // fArm 伺服舵机连接引脚9  舵机代号'f'
  delay(200);          // 稳定性等待
  claw.attach(10);      // claw 伺服舵机连接引脚6  舵机代号'c'
  delay(200);          // 稳定性等待
 
  base.write(90); 
  delay(10);
  fArm.write(90); 
  delay(10);
  rArm.write(90); 
  delay(10);
  claw.write(60);  
  delay(10); 

  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  Serial.begin(9600); 
  Serial.println("Welcome to Taichi-Maker Robot Arm Tutorial");   
}
 
 
void loop(){
    ultrasound();
  if (Serial.available()>0) {  
    char serialCmd = Serial.read();
    armDataCmd(serialCmd);
  }
  if(cm<=DES_MAX&&cm>=DES_MIN)
  { 
    //if(Flag==Free)
   Actionsetting(); 
  }
}
 
void armDataCmd(char serialCmd){ //Arduino根据串行指令执行相应操作
                                 //指令示例：b45 底盘转到45度角度位置
                                 //          o 输出机械臂舵机状态信息
  if (serialCmd == 'b' || serialCmd == 'c' || serialCmd == 'f' || serialCmd == 'r'){
    int servoData = Serial.parseInt();
    servoCmd(serialCmd, servoData, DSD);  // 机械臂舵机运行函数（参数：舵机名，目标角度，延迟/速度）
  } else {
    switch(serialCmd){    
      case 'o':  // 输出舵机状态信息
       // reportStatus();
       Actionsetting();
        break;
      default:  //未知指令反馈
        Serial.println("Unknown Command.");
    }
  }  
}
 
void servoCmd(char servoName, int toPos, int servoDelay){  
  Servo servo2go;  //创建servo对象
 
  //串口监视器输出接收指令信息
  Serial.println("");
  Serial.print("+Command: Servo ");
  Serial.print(servoName);
  Serial.print(" to ");
  Serial.print(toPos);
  Serial.print(" at servoDelay value ");
  Serial.print(servoDelay);
  Serial.println(".");
  Serial.println("");  
   
  int fromPos; //建立变量，存储电机起始运动角度值
   
  switch(servoName){
    case 'b':
      if(toPos >= baseMin && toPos <= baseMax){
        servo2go = base;
        fromPos = base.read();  // 获取当前电机角度值用于“电机运动起始角度值”
        break;
      } else {
        Serial.println("+Warning: Base Servo Value Out Of Limit!");
        return;
      }
  
    case 'c':
      if(toPos >= clawMin && toPos <= clawMax){    
        servo2go = claw;
        fromPos = claw.read();  // 获取当前电机角度值用于“电机运动起始角度值”
        break;
      } else {
        Serial.println("+Warning: Claw Servo Value Out Of Limit!");
        return;        
      }
 
    case 'f':
      if(toPos >= fArmMin && toPos <= fArmMax){
        servo2go = fArm;
        fromPos = fArm.read();  // 获取当前电机角度值用于“电机运动起始角度值”
        break;
      } else {
        Serial.println("+Warning: fArm Servo Value Out Of Limit!");
        return;
      }
          
    case 'r':
      if(toPos >= rArmMin && toPos <= rArmMax){
        servo2go = rArm;
        fromPos = rArm.read();  // 获取当前电机角度值用于“电机运动起始角度值”
        break;
      } else {
        Serial.println("+Warning: rArm Servo Value Out Of Limit!");
        return;
      }      
  }
 
  //指挥电机运行
  if (fromPos <= toPos){  //如果“起始角度值”小于“目标角度值”
    for (int i=fromPos; i<=toPos; i++){
      servo2go.write(i);
      delay (servoDelay);
    }
  }  else { //否则“起始角度值”大于“目标角度值”
    for (int i=fromPos; i>=toPos; i--){
      servo2go.write(i);
      delay (servoDelay);
    }
  }
}
 
void reportStatus(){  //舵机状态信息
  Serial.println("");
  Serial.println("");
  Serial.println("+ Robot-Arm Status Report +");
  Serial.print("Claw Position: "); Serial.println(claw.read());
  Serial.print("Base Position: "); Serial.println(base.read());
  Serial.print("Rear  Arm Position:"); Serial.println(rArm.read());
  Serial.print("Front Arm Position:"); Serial.println(fArm.read());
  Serial.println("++++++++++++++++++++++++++");
  Serial.println("");
}
void Actionsetting()
{
     servoCmd('c',180,DSD);
     delay(1000);
     servoCmd('r',130,DSD);
     delay(1000);
     servoCmd('f',35,DSD);
     delay(1000);
     servoCmd('c',60,DSD);
     delay(1000);
     servoCmd('r',85,5);
     delay(1000);
     servoCmd('f',100,5);
     Flag=Hang;
}
void ultrasound()
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(5);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(Echo, HIGH);
 
  // convert the time into a distance
  cm = (duration/2) / 29.1;
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  delay(1000);
}
