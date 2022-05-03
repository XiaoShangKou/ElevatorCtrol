/*
 * 逆为正，顺为负数
 2000步为 9cm.
 所以1步为0.005cm
 所以24cm 为 4800步（逆）
 9.5cm  为1900步（逆）
 
*/
#include <AccelStepper.h>  //本示例程序使用AccelStepper库
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

/*********************人脸识别部分*********************/
int faceFlag=0;//人脸识别标志位
HUSKYLENS huskylens;
//A1-Hold引脚 A2-Resume引脚 
SoftwareSerial mySerial(A1, A2); // RX, TX

//存储的用户信息
//索引为用户代号
//用户的第一个参数是否进入电梯，第二个参数用户对应的电梯
int YongHu[4][2]={
{0,1},{0,2},{0,3},{0,4}  
};
float waitTime=0;//等待时间
/*********************步进电机部分*********************/
// 定义电机控制用常量
const int enablePin = 8;  // 使能控制引脚
 
const int xdirPin = 5;     // x方向控制引脚
const int xstepPin = 2;    // x步进控制引脚
const int ydirPin = 6;     // y方向控制引脚
const int ystepPin = 3;    // y步进控制引脚
const int zdirPin = 7;     // z方向控制引脚
const int zstepPin = 4;    // z步进控制引脚
 
const int moveSteps = 1900;    //测试电机运行使用的运行步数
 
AccelStepper stepper1(1,xstepPin,xdirPin);//建立步进电机对象1
AccelStepper stepper2(1,ystepPin,ydirPin);//建立步进电机对象2

const int Reset[2]={2900,6700};//归位点位置
int ResetFlag=0;//归位的标志位
/*
楼层按钮与归位点之间的距离/0.005
*/
const int Floor[6][2]={
  {-1460,-4020},{-2740,-2740},
  {-600,-3160},{-1880,-1880},
  {260,-2300},{-1020,-1020}  
};

int Jieshu=0;//电梯点击结束标志位

/*********************舵机部分*********************/
//11-Z+引脚
int servopin=11;//定义数字接口A0 连接伺服舵机信号线
int myangle;//定义角度变量0-180
int pulsewidth;//定义脉宽变量

/*********************语言模块部分*********************/

char incomedate=0;
int yuYinFlag=0;//语言识别到用户楼层的标志位



/////////////////////////////////步进电机部分/////////////////////////////////
/*
*用途:归位到距原点长 24cm 宽 9.5cm
*参数:无
*返回值:无
*原理:左电机逆时针4800步 顺时针1900 所以为:逆时针2900
* 右电机逆时针4800步 逆时针1900 所以为:逆时针6700
* 时间：2022/4/27
* 修改：不再单独移动一个轴，而是一起移动
*/
void GuiWei(){
  // 控制步进电机1逆时针3600
  if ( stepper1.currentPosition() == 0 ){ 
    stepper1.moveTo(Reset[0]);              
  } 
  // 控制步进电机2逆时针7400
  if ( stepper2.currentPosition() == 0 ){ 
    stepper2.moveTo(Reset[1]);              
  }

  //当到归位点的时候，将归位的标志位置为一
  if(stepper1.currentPosition()==Reset[0] && stepper2.currentPosition()==Reset[1]){
    ResetFlag=1;     
  }
}

/*
用处:去往对应的楼层
参数:int LouCeng 那个楼层
返回值:无
时间：2022/4/27
修改：无
*/
void GoTo(int LouCeng){
  LouCeng--;
  //5.1cm 顺1020
  // 控制步进电机1逆时针3600
  if ( stepper1.currentPosition() == Reset[0] && stepper2.currentPosition() == Reset[1]){ 
    stepper1.move(Floor[LouCeng][0]);    
    stepper2.move(Floor[LouCeng][1]);      
       
  } 

  //到达对应的楼层
  else if( stepper2.currentPosition()==Reset[1]+Floor[LouCeng][1] && stepper1.currentPosition()==Reset[0]+Floor[LouCeng][0] ){
      Click();//舵机按下    
      stepper1.move(-Floor[LouCeng][0]);
      stepper2.move(-Floor[LouCeng][1]);
      YongHu[LouCeng][0]=0;
      incomedate=0;
      Jieshu=1;
  }
 
}



/////////////////////////////////舵机部分/////////////////////////////////
/*
用处:舵机响应
参数:int servopin,int myangle 引脚，角度
返回值：无
时间：2022/4/19
修改：无
*/
void servopulse(int servopin,int myangle)//定义一个脉冲函数
{
  pulsewidth=(myangle*11)+500;//将角度转化为500-2480 的脉宽值
  
  digitalWrite(servopin,HIGH);//将舵机接口电平至高
  
  delayMicroseconds(pulsewidth);//延时脉宽值的微秒数weimiao
  
  digitalWrite(servopin,LOW);//将舵机接口电平至低
  
  delay(20-pulsewidth/1000);
}
/*
用处:点击电梯
参数:无
返回值:无
时间：2022/4/19
修改：无
*/
void Click(){
  delay(200);
  //点击电梯
  for(int i=0;i<=50;i++) //给予舵机足够的时间让它转到指定角度
  {
    servopulse(servopin,150);//引用脉冲函数
  }
  delay(200);//给予舵机足够的时间让它转到指定角度
  for(int i=0;i<=50;i++) //给予舵机足够的时间让它转到指定角度
  {
    servopulse(servopin,90);//引用脉冲函数
  }  
}


/////////////////////////////////人脸部分/////////////////////////////////
/*
用处:识别相应的用户是否进入电梯
参数:HUSKYLENSResult result 人脸显示对象
返回值：无
时间：2022/4/27
修改：适配为Ardiuno
*/
void ResultCtrol(HUSKYLENSResult result){
  if (result.command == COMMAND_RETURN_BLOCK){//如果识别的是块(识别到人)

     if(result.ID==1){//如果识别到ID为1的用户
         Serial.println(F("shibiedao1"));
         YongHu[0][0]=1;
     }
     if(result.ID==2){//如果识别到ID为2的用户
         Serial.println(F("shibiedao2"));
         YongHu[1][0]=1;
     }
     if(result.ID==3){//如果识别到ID为3的用户
         Serial.println(F("shibiedao3"));
         YongHu[2][0]=1;
     }
     if(result.ID==4){//如果识别到ID为4的用户
         Serial.println(F("shibiedao4"));
         YongHu[3][0]=1;
     }
     if(YongHu[0][0]==1 ||YongHu[1][0]==1 || YongHu[2][0]==1 || YongHu[3][0]==1){
         faceFlag=1;
     }
      
  }
}

/*
用处:去往识别到用户的楼层
参数:无
返回值：无
时间：2022/4/28
修改：减少了臃肿
*/
void FaceGoto(){
//  Serial.println(YongHu[1][0]); 
  if(YongHu[0][0]){
    GoTo(YongHu[0][1]);   
  }
  else if(YongHu[1][0]){
    GoTo(YongHu[1][1]);   
  }
  else if(YongHu[2][0]){
    GoTo(YongHu[2][1]);   
  }
  else if(YongHu[3][0]){
    GoTo(YongHu[3][1]);   
  }

  //如果按下电梯并归位，再次开启人脸识别
  if (stepper1.currentPosition() == Reset[0] && stepper2.currentPosition() == Reset[1]){
    if(Jieshu==1) {
        faceFlag=0;
        waitTime=0;
        Jieshu=0;
        
    } 
  }
  
}



/////////////////////////////////语言识别部分/////////////////////////////////
/*
用途:通过语音内容滑杆点击那个楼层的按钮（也可以是esp32手机点击的楼层）
参数: char incomedate 语音或者手机串口发送过来的内容
返回值:暂无
修改:将语音模块接入到esp32中，因此此函数也是手机发送消息控制滑杆系统的
时间:2022/5/1
*/
int SayGoto(char incomedate){
//  Serial.println(incomedate);
  if(incomedate=='1'){
    GoTo(1);
  }
  else if(incomedate=='2'){
    GoTo(2);
  }
  else if(incomedate=='3'){
    GoTo(3);
  }
  else if(incomedate=='4'){
    GoTo(4);
  }
  else if(incomedate=='5'){
    GoTo(5);
  }
  else if(incomedate=='6'){
    GoTo(6);
  }
  if(incomedate=='1' || incomedate=='2' ||incomedate=='3' ||incomedate=='4' ||incomedate=='5' ||incomedate=='6'){
      yuYinFlag=1;
      waitTime=0;
  }
  //如果按下电梯并归位，再次开启语音识别
  if (stepper1.currentPosition() == Reset[0] && stepper2.currentPosition() == Reset[1]){
    if(Jieshu==1) {
        yuYinFlag=0;
        Jieshu=0;
       
    } 
  }
}


void setup() {
  Serial.begin(9600);
  Serial.println("chuankouInit");
  
  /////////////////////////////////语言识别部分/////////////////////////////////
 
  /*已经接入ESP32*/
  
  /////////////////////////////////人脸部分/////////////////////////////////
  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
  {
      Serial.println(F("Begin failed!"));
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }



  /////////////////////////////////步进电机部分/////////////////////////////////
  pinMode(xstepPin,OUTPUT);     // Arduino控制A4988x步进引脚为输出模式
  pinMode(xdirPin,OUTPUT);      // Arduino控制A4988x方向引脚为输出模式
  pinMode(ystepPin,OUTPUT);     // Arduino控制A4988y步进引脚为输出模式
  pinMode(ydirPin,OUTPUT);      // Arduino控制A4988y方向引脚为输出模式
 
  
  pinMode(enablePin,OUTPUT);   // Arduino控制A4988使能引脚为输出模式
  digitalWrite(enablePin,LOW); // 将使能控制引脚设置为低电平从而让
                               // 电机驱动板进入工作状态
                                
  stepper1.setMaxSpeed(2700.0);     // 设置电机最大速度2700
  stepper1.setAcceleration(700.0);  // 设置电机加速度700.0  
  stepper2.setMaxSpeed(2700.0);     // 设置电机最大速度2700 
  stepper2.setAcceleration(700.0);  // 设置电机加速度700.0 


  //////////////////
  ///////////////舵机部分/////////////////////////////////
  pinMode(servopin,OUTPUT);//设定舵机接口为输出接口

  
}
 
void loop() { 
  GuiWei();//滑杆归位
  
  if(ResetFlag){
    /////////////////////////////////语言识别部分/////////////////////////////////
    
    if(yuYinFlag==0){
      
      if (Serial.available() > 0)//串口接收到数据
      {
      incomedate = Serial.read();//获取串口接收到的数据
      Serial.println(incomedate);
      }

     
    }
    
    /////////////////////////////////人脸识别部分/////////////////////////////////
    
    
    if(faceFlag==0 && yuYinFlag==0){
      if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
      if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
      if(huskylens.available())//如果识别到信息
        {
            
            HUSKYLENSResult result = huskylens.read();
            ResultCtrol(result);//通过人脸信息响应舵机   
        }
      
    }
    //识别到人脸等待一段时间看用户是否前往默认楼层
    if(faceFlag==1){
      waitTime+=0.1;
    }
    if(waitTime>10000 && yuYinFlag==0){
      FaceGoto();
    }
    else{
      SayGoto(incomedate);
    }
    
  }
   
  stepper1.run();   // 1号电机运行
  stepper2.run();   // 2号电机运行
 
}
