#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>   

//////////////////////////////网络部分//////////////////////////////
//网络的名称密码
const char *AP_SSID="小伤口";
const char *AP_Password="12345678";

const int DianTipin=14;//5引脚

WebServer server(80);//80端口



char incomedate=0;


//////////////////////////////传输部分//////////////////////////////
//我们测试需要用到的串口，ESP32一共有3个串口，串口0应该是用于程序烧写和打印输出调试用的，所以我们只用1和2
HardwareSerial MySerial(1);  


//根目录的html
void handleRoot(){
  String HTML="<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"><meta name=\"viewport\" content=\"width=device-width\" initial-scale=\"1.0\"><title>电梯控制</title><style>.hole{width:200px;height:100px;margin:0 auto}.hole p{text-align:center}.hole h1{text-align:center}</style><body><div class=\"hole\"><h1>电梯控制</h1><p>success</p><p>校验用户要去的楼层:</p></div>";
  //网页内容显示灯的状态
  
  server.send(200,"text/html",HTML);//传输HTML 成功200 失败404
  
}
/*
用途:通过get请求发送对应的串口消息，来达到控制滑杆的目的
参数:无
返回值:无
修改:无
时间:2022/5/1
*/
void ctrol(){
  String state=server.arg("IO");//获取网址中IO参数的值（get请求）
  if(state=="1"){
    Serial.print(state);
    MySerial.println(state);

    
  }else if(state=="2"){
    Serial.print(state);
    MySerial.println(state); 
   
  }
  else if(state=="3"){
    Serial.print(state);
    MySerial.println(state); 
  }
  else if(state=="4"){
    Serial.print(state);
    MySerial.println(state); 
  }
  else if(state=="5"){
    Serial.print(state);
    MySerial.println(state); 
  }
  else if(state=="6"){
    Serial.print(state);
    MySerial.println(state); 
  }

  //网页内容显示灯的状态
  server.send(200,"text/html","DianTi is <b>"+state+"</b>.");
  
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//串口波特率
 
 //串口的开启，这里还可以传一些别的参数，但是我们只传入下面四个最重要的：波特率，默认SERIAL_8N1为8位数据位、无校验、1位停止位，后面两个分别为 RXD,TXD 引脚
  MySerial.begin(9600, SERIAL_8N1, 14, 12);


  /////////////////////////网络连接/////////////////////////
  WiFi.softAP(AP_SSID,AP_Password);  //设置AP模式热点的名称和密码，密码可不填则发出的热点为无密码热点

  Serial.print("\n ESP32建立的wifi名称为：");
  Serial.print(AP_SSID);  //串口输出ESP32建立的wifi的名称
  Serial.print("\nESP32建立wifi的IP地址为：");
  Serial.print(WiFi.softAPIP());  //串口输出热点的IP地址
  

   /////////////////////////Web部分/////////////////////////
  //创建服务器(/这个是根目录)
  server.on("/",handleRoot);
  server.on("/ctrol",ctrol);//get请求 预约电梯
  
  server.onNotFound([](){server.send(200,"text.html","404 NotFound");});//404找不到
  
  server.begin();//开启服务器
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();//处理客户端的连接
  if (MySerial.available() > 0)//串口接收到数据
  {
    incomedate = MySerial.read();//获取串口接收到的数据
    Serial.println(incomedate);
    MySerial.println(incomedate);//发送数据到软串口
  }
}
