#include <OneWire.h>//引入水溫感應函式庫
#include <DallasTemperature.h> //引入水溫感應函式庫
#include <SPI.h>//引入通訊函式庫
#include <RH_RF95.h>//引入LoRa函式庫
#define _baudrate   9600 //定義鮑率
#define DQ_Pin 6//定義水溫感測器針腳
const int soilsensorPin=2;//定義土壤濕度感應器腳位
const int soilmotorPin=3;//定義土壤澆水馬達腳位
const int watermotorPin=4;//定義淨水器抽水馬達腳位
const int waterhighPin=0;//定義水位感應器腳位
const int senosensorPin=4;//定義水質濁度感應器腳位
int datalen;//定義資料長度
OneWire oneWire(DQ_Pin);//定義水溫感測腳位
DallasTemperature sensors(&oneWire);//定義水溫感測參數

RH_RF95 rf95;//定義縮寫

void setup()
{
  pinMode(soilsensorPin,INPUT);//土壤濕度感應器腳位設定為輸入
  pinMode(soilmotorPin, OUTPUT);//土壤澆水馬達腳位設定為輸出  
  pinMode(watermotorPin, OUTPUT);//淨水器抽水馬達腳位設定為輸出  
  Serial.begin(_baudrate);//設定初始化鮑率
  sensors.begin();//水溫感測器初始化
  if (!rf95.init())
    Serial.println("LoRa初始化失敗");//初始化LoRa模組（若失敗則印出LoRa初始化失敗）
}
 
void loop()
{
  int senosensorValue = analogRead(A4);//定義濁度讀入腳位
  int volt = 0;//定義警告狀態變數正常=0,異常=1
  float voltage = senosensorValue * (5.0 / 1024.0); // 將讀入數值作轉換方便後續使用 
  Serial.print("濁度-->");
  Serial.println(voltage); //監視器顯示濁度數值
  if(voltage > 3.5){
    digitalWrite(watermotorPin,HIGH);//水質濁度大於3.5則將水抽入淨水器
    int volt = 1;//水質異常，更改變數為1。
  }else{
    digitalWrite(watermotorPin,LOW);//水質乾淨則不需抽水
    int volt = 0;//水質正常，不需通知，更改變數為0
  }
  String vol = String(volt);//轉換型態，方便後續LoRa發送

  int moist;//變數土壤濕度int moist;
  int moi = 0;//定義警告狀態變數正常=0,異常=1
  moist = analogRead(soilsensorPin);//定義土壤濕度讀入腳位
  Serial.print("土壤濕度-->");
  Serial.println(moist);//監視器顯示土壤乾燥數值
  if (moist > 800) {
       digitalWrite(soilmotorPin,HIGH); //土壤乾燥程度大於800則將水抽出灌溉
       moi = 1;//濕度異常，更改變數為1。
  }else{
      digitalWrite(soilmotorPin,LOW);  //土壤乾燥程度低於800則不需抽水
        moi = 0;//濕度正常，不需通知，更改變數為0
   }
   String mois = String(moi);//轉換型態，方便後續LoRa發送
 
  
  sensors.requestTemperatures();//呼叫溫度感測函數
  Serial.print("水溫-->");
  Serial.println(sensors.getTempCByIndex(0));//監視器顯示溫度
  int water_tmp = sensors.getTempCByIndex(0);
  int temh = 0;//定義警告狀態變數正常=0,異常=1
  if(sensors.getTempCByIndex(0)<20){
    temh = 1;//溫度異常，更改變數為1。
  }else if(sensors.getTempCByIndex(0)>20 && sensors.getTempCByIndex(0)<30){
    temh = 0;
  }else if(sensors.getTempCByIndex(0)>30){
    temh = 1;//溫度異常，更改變數為1。
  }
  String tem = String(temh);//轉換型態，方便後續LoRa發送
  
  
  int waterhigh;//變數水位高度
  int whl = 0;//定義警告狀態變數正常=0,異常=1
  waterhigh = analogRead(waterhighPin);//定義水位讀入腳位
  Serial.print("水位高度-->");
  Serial.println(waterhigh);//監視器顯示水位數值        
  if (waterhigh > 480) {
    Serial.println("High Level");//若水位數值大於480則監視器顯示High level 
    whl=1;//水位異常，更改變數為1。
  }          
  else if ((waterhigh > 340) && (waterhigh <= 480)) {
    Serial.println("Middle Level");//若水位數值介於340~480間則監視器顯示Middle level
    whl=0;//水位正常，不需通知，更改變數為0
  }
  else if ((waterhigh > 100) && (waterhigh <=340)){
    Serial.println("Low Level");//若水位數值介於100~340間則監視器顯示low level
    whl=1;//水位異常，更改變數為1。
  }     
  else if (waterhigh <=100){
    Serial.println("NO Water");//若水位數值低於100則監視器顯示No water
    whl=1;//水位異常，更改變數為1。
  }
  String wh = String(whl);//轉換型態，方便後續LoRa發送

  //上傳至物聯網監測網站
  String data =  vol + " " + moi + " " + tem + " " + wh + " ";//將資料打包成一串
  Serial.print(data);//將資料印出檢查
  datalen = data.length()+1;//確定資料長度
  char d[datalen];//建立資料串
  data.toCharArray(d, datalen); //字串轉陣列
  Serial.println("Sending to rf95_server");//印出發送訊息檢查
  rf95.send(d, sizeof(d));//發送資料
  rf95.waitPacketSent();//等待資料送出

  delay(10000);//10秒鐘重新檢查一次
}
