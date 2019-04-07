#define preH 35
#define aftH 5

#define FASTADC 1
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <SoftwareSerial.h> //BLE
SoftwareSerial bleSerial(13,16);  //TX,RX

#include <Wire.h> //gyro-accel, I2C

#include "MPU6050.h"  //MPU-offset
MPU6050 accelgyro(0x68);

long aX[preH+aftH], aY[preH+aftH], aZ[preH+aftH]; //accel-value
float g0[3];  //gravity force-m/s^2; X,Y,Z  //valueInFirstVibration
float g1[3];  //valueInSecondVibration

long gyX[preH+aftH], gyY[preH+aftH], gyZ[preH+aftH];  //gyro-value
float r0[3];  //rotation-deg/s; X,Y,Z //valueInFirstVibration
float r1[3];  //valueInSecondVibration

long tempValue;
float temperature;

byte piezo[preH+aftH]; //piezo value
byte piezoH[3];  //threeHighestVibration
byte piezoHindex[3]; //indexOfHighestVibration

const int piezoPin = A0;
const int piezoThreshold = 15;

int countStrokes;
byte indexOfFirst; //indexOfFirstVibration
bool hitVibration;
String outputStroke;

void setup()
{
  Serial.begin(115200);
  Serial.println("PC connected");
  
  #if FASTADC //adc_prescale: 8, 316 page 32U4 datasheet
    cbi(ADCSRA,ADPS2);  //0
    sbi(ADCSRA,ADPS1);  //1
    sbi(ADCSRA,ADPS0);  //1
  #endif
  
  bleSerial.begin(9600);
  bleSerial.println("Phone connected");
  
  pinMode(piezoPin,INPUT);
  
  Wire.begin();
  setupMPU();
  delay(10);
  setOffset();
  delay(10);
  
  countStrokes = 0;
  indexOfFirst = 0;
  hitVibration = false;
}

void loop()
{
  preHitRecord();
  if (hitVibration)
  {
    aftHitRecord();
    
    countStrokes++;
    printRawBLE();
    analyzeHit();

    bleSerial.println(outputStroke);
    bleSerial.println();

    Serial.println(outputStroke);
    Serial.println();
    
    if (countStrokes%10 == 0)
      getTemperature();
      
    aftHitClean();    
    delay(500);
  }
}

void setupMPU()
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x6B);
  Wire.write(0b00000000); //power mode
  Wire.endTransmission();

  Wire.beginTransmission(0b1101000);
  Wire.write(0x1B);
  Wire.write(0b00011000); //gyro scale: +/- 2000deg/s
  Wire.endTransmission();

  Wire.beginTransmission(0b1101000);
  Wire.write(0x1C);
  Wire.write(0b00011000); //acceleration scale: +/- 16g
  Wire.endTransmission();
}
void setOffset()
{
  accelgyro.setXAccelOffset(565);
  accelgyro.setYAccelOffset(2527);
  accelgyro.setZAccelOffset(1546);
  accelgyro.setXGyroOffset(90);
  accelgyro.setYGyroOffset(-45);
  accelgyro.setZGyroOffset(33);
}
inline void preHitRecord()
{
  for(int i = 0 ; i < preH ; i++)
  {
    recordGyroRegisters(i);    
    recordAccelRegisters(i);
    piezo[i] = analogRead(piezoPin);
    if (piezo[i] >= piezoThreshold) hitVibration = true;
  }
}
inline void aftHitRecord()
{
  for(int i = preH ; i < (preH + aftH) ; i++)
  {
    recordGyroRegisters(i);    
    recordAccelRegisters(i);
    piezo[i] = analogRead(piezoPin);
  }
  processData();
}
inline void recordAccelRegisters(int j)
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6);
  while(Wire.available() < 6);
  aX[j] = Wire.read()<<8|Wire.read();
  aY[j] = Wire.read()<<8|Wire.read();
  aZ[j] = Wire.read()<<8|Wire.read();
}
inline void recordGyroRegisters(int j)
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6);
  while(Wire.available() < 6);
  gyX[j] = Wire.read()<<8|Wire.read();
  gyY[j] = Wire.read()<<8|Wire.read();
  gyZ[j] = Wire.read()<<8|Wire.read();
}
void processData()
{
  bool firstFound = false;
  for(int i = 0 ; i < (preH + aftH) ; i++)
  {
    if(!firstFound && piezo[i] >= piezoThreshold)
    {
      indexOfFirst = i;
      firstFound = true;
    }
    if(piezo[i] > piezoH[0])
    {
      piezoH[2] = piezoH[1];
      piezoH[1] = piezoH[0];
      piezoH[0] = piezo[i];
    
      piezoHindex[2] = piezoHindex[1];
      piezoHindex[1] = piezoHindex[0];
      piezoHindex[0] = i;
    }
    else if (piezo[i] > piezoH[1])
    {
      piezoH[2] = piezoH[1];
      piezoH[1] = piezo[i];
    
      piezoHindex[2] = piezoHindex[1];
      piezoHindex[1] = i;
    }
    else if (piezo[i] > piezoH[2])
    {
      piezoH[2] = piezo[i];
    
      piezoHindex[2] = i;
    }
  }
  
  g0[0] = aX[indexOfFirst] / 2048.0;
  g0[1] = aY[indexOfFirst] / 2048.0;
  g0[2] = aZ[indexOfFirst] / 2048.0;

  r0[0] = gyX[indexOfFirst] / 16.4;
  r0[1] = gyY[indexOfFirst] / 16.4;
  r0[2] = gyZ[indexOfFirst] / 16.4;
  
  g1[0] = aX[indexOfFirst+1] / 2048.0;
  g1[1] = aY[indexOfFirst+1] / 2048.0;
  g1[2] = aZ[indexOfFirst+1] / 2048.0;

  r1[0] = gyX[indexOfFirst+1] / 16.4;
  r1[1] = gyY[indexOfFirst+1] / 16.4;
  r1[2] = gyZ[indexOfFirst+1] / 16.4;
}
void aftHitClean()
{
  hitVibration = false;
  indexOfFirst = 0;
  for(int i = 0 ; i < 3 ; i++)
  {
    piezoH[i] = 0;
    piezoHindex[i] = 0;
  }
}
void getTemperature()
{
  Wire.beginTransmission(0b1101000);
  Wire.write(0x41);
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,2);
  while(Wire.available() < 2);
  tempValue = Wire.read()<<8|Wire.read();
  temperature = (tempValue/340.0)+36.53;

  bleSerial.print("temp: ");
  bleSerial.print(temperature);
  bleSerial.println(" oC");
  bleSerial.println();
}
void printRawBLE()
{
  bleSerial.print("br: "); //broj_udarca
  bleSerial.println(countStrokes);

  bleSerial.print("vbrH: "); //vibracija
  for(int i = 0 ; i < 3 ; i++)
  {
    bleSerial.print(piezoH[i]);
    bleSerial.print("[");
    bleSerial.print(piezoHindex[i]);
    if(i == 2)
      bleSerial.println("]");
    else
      bleSerial.print("],");
  }
  bleSerial.print("vbrF: "); //vibracija
  for(int i = 0 ; i < 3 ; i++)
  {
    bleSerial.print(piezo[indexOfFirst+i]);
    bleSerial.print("[");
    bleSerial.print(indexOfFirst+i);
    if(i == 2)
      bleSerial.println("]");
    else
      bleSerial.print("],");
  }
  
  bleSerial.print("rotF[");  //rotacija
  bleSerial.print(indexOfFirst);
  bleSerial.print("]: ");
  for(int i = 0 ; i < 3 ; i++)
  {
    if(i == 2)
      bleSerial.println(r0[i]);
    else
    {
      bleSerial.print(r0[i]);
      bleSerial.print(",");
    }
  }
  
  bleSerial.print("aclF[");  //akceleracija
  bleSerial.print(indexOfFirst);
  bleSerial.print("]: ");
  for(int i = 0 ; i < 3 ; i++)
  {
    if(i == 2)
      bleSerial.println(g0[i]);
    else
    {
      bleSerial.print(g0[i]);
      bleSerial.print(",");
    }
  }

  bleSerial.print("rotF[");  //rotacija
  bleSerial.print(indexOfFirst+1);
  bleSerial.print("]: ");
  for(int i = 0 ; i < 3 ; i++)
  {
    if(i == 2)
      bleSerial.println(r1[i]);
    else
    {
      bleSerial.print(r1[i]);
      bleSerial.print(",");
    }
  }

  bleSerial.print("aclF[");  //akceleracija
  bleSerial.print(indexOfFirst+1);
  bleSerial.print("]: ");
  for(int i = 0 ; i < 3 ; i++)
  {
    if(i == 2)
      bleSerial.println(g1[i]);
    else
    {
      bleSerial.print(g1[i]);
      bleSerial.print(",");
    }
  }
}
void analyzeHit()
{
  outputStroke = "udr: ";
  //fh-sp
  if((r0[0]>(600.0) || r1[0]>(600.0)) && ((r0[1]>(-1700.0) && r0[1]<(200.0)) || (r1[1]>(-1200.0) && r1[1]<(200.0))) && ((r0[2]>(-1200.0) && r0[2]<(-300.0)) || (r1[2]>(-1700.0) && r1[2]<(-300.0))) && (g0[0]<(1.0) || g1[0]<(3.0) || g0[0]>(15.7) || g1[0]>(15.7)) && (g0[2]<(-1.0) || g1[2]<(-1.0) || g0[2]>(15.7) || g1[2]>(15.7)) && piezoH[0]>(30))
    outputStroke += "fh-sp, ";
  //fh-fl
  if(r0[0]>(900.0) && r0[1]>(-1200.0) && r0[1]<(-400.0) && r0[2]>(-1400.0) && r0[2]<(-700.0) && ((g0[0]>(-12.0) && g0[0]<(-5.0)) || g0[0]>(15.7) || g0[0]<(-15.7) || g1[0]>(15.7) || g1[0]<(-15.7)) && ((g0[1]>(-12.0) && g0[1]<(0.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && (g0[2]<(-5.0) || g0[2]>(15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(50))
    outputStroke += "fh-fl, ";
  //fh-sl
  if(((r0[0]>(600.0) && r0[0]<(1700.0)) || (r1[0]>(600.0) && r1[0]<(1700.0))) && ((r0[1]>(-1200.0) && r0[1]<(0.0)) || (r1[1]>(-1200.0) && r1[1]<(0.0))) && ((r0[2]>(400.0) && r0[2]<(1400.0)) || (r1[2]>(400.0) && r1[2]<(1400.0))) && (g0[0]>(2.0) || g1[0]>(2.0) || g0[0]<(-15.7) || g1[0]<(-15.7)) && ((g0[1]>(-5.0) && g0[1]<(5.0)) || (g1[1]>(-5.0) && g1[1]<(5.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && ((g0[2]>(-8.0) && g0[2]<(4.0)) || (g1[2]>(-8.0) && g1[2]<(4.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(40))
    outputStroke += "fh-sl, ";
  //bk-sp
  if((r0[0]<(-500.0) || r1[0]<(-500.0)) && ((r0[1]>(-500.0) && r0[1]<(500.0)) || (r1[1]>(-500.0) && r1[1]<(500.0))) && ((r0[2]>(-700.0) && r0[2]<(-100.0)) || (r1[2]>(-700.0) && r1[2]<(-100.0))) && ((g0[0]>(1.0) && g0[0]<(9.0)) || (g1[0]>(1.0) && g1[0]<(9.0)) || g0[0]>(15.7) || g0[0]<(-15.7) || g1[0]>(15.7) || g1[0]<(-15.7)) && (g0[1]<(1.0) || g1[1]<(1.0) || g0[1]>(15.7) || g1[1]>(15.7)) && ((g0[2]>(-8.0) && g0[2]<(2.0)) || (g1[2]>(-8.0) && g1[2]<(0.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(25))
    outputStroke += "bk-sp, ";
  //bk-fl
  if(r0[0]<(-900.0) && r0[1]>(-900.0) && r0[1]<(400.0) && r0[2]>(-500.0) && r0[2]<(250.0) && ((g0[0]>(3.0) && g0[0]<(10.0)) || g0[0]>(15.7) || g0[0]<(-15.7) || g1[0]>(15.7) || g1[0]<(-15.7)) && ((g0[1]>(-12.0) && g0[1]<(-2.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && ((g0[2]>(-6.0) && g0[2]<(3.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(50))
    outputStroke += "bk-fl, ";
  //bk-sl
  if(((r0[0]>(-1300.0) && r0[0]<(-300.0)) || (r1[0]>(-1300.0) && r1[0]<(-300.0))) && ((r0[1]>(-1000.0) && r0[1]<(1000.0)) || (r1[1]>(-1000.0) && r1[1]<(1000.0))) && ((r0[2]>(800.0) && r0[2]<(1600.0)) || (r1[2]>(800.0) && r1[2]<(1600.0))) && ((g0[0]>(2.0) && g0[0]<(10.0)) || (g1[0]>(2.0) && g1[0]<(10.0)) || g0[0]>(15.7) || g0[0]<(-15.7) || g1[0]>(15.7) || g1[0]<(-15.7)) && ((g0[1]>(-9.0) && g0[1]<(3.0)) || (g1[1]>(-9.0) && g1[1]<(3.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && ((g0[2]>(-4.0) && g0[2]<(6.0)) || (g1[2]>(-4.0) && g1[2]<(6.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)))
    outputStroke += "bk-sl, ";
  //sr-sl
  if((r0[0]>(1800.0) || r1[0]>(1700.0)) && r0[1]>(-1600.0) && r0[1]<(-700.0) && ((r0[2]>(400.0) && r0[2]<(1100.0)) || (r1[2]>(300.0) && r1[2]<(1100.0))) && (g0[0]>(3.0) || g1[0]>(3.0) || g0[0]<(-15.7) || g1[0]<(-15.7)) && ((g0[1]>(-10.0) && g0[1]<(10.0)) || (g1[1]>(-10.0) && g1[1]<(10.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && ((g0[2]>(-10.0) && g0[2]<(10.0)) || (g1[2]>(-10.0) && g1[2]<(10.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(40))
    outputStroke += "sr-sl, ";
  //sr-fl
  if((r0[0]>(1800.0) || r1[0]>(1700.0)) && r0[1]>(-1800.0) && r0[1]<(-1000.0) && r0[1]<r1[1] && ((r0[2]>(-600.0) && r0[2]<(100.0)) || (r1[2]>(-700.0) && r1[2]<(100.0))) && ((g0[0]>(4.0) && g0[0]<(14.0)) || g0[0]>(15.7) || g0[0]<(-15.7) || g1[0]>(15.7) || g1[0]<(-15.7)) && ((g0[1]>(-8.0) && g0[1]<(8.0)) || (g1[1]>(-8.0) && g1[1]<(8.0)) || g0[1]>(15.7) || g0[1]<(-15.7) || g1[1]>(15.7) || g1[1]<(-15.7)) && ((g0[2]>(-13.0) && g0[2]<(-3.0)) || g0[2]>(15.7) || g0[2]<(-15.7) || g1[2]>(15.7) || g1[2]<(-15.7)) && piezoH[0]>(50))
    outputStroke += "sr-fl, ";
  //voleji
  if (outputStroke.length() < 7)
  {
    //vl-fh
    if(((r0[0]>(100.0) && r0[0]<(1500.0)) || (r1[0]>(100.0)  && r1[0]<(1500.0))) && r0[1]>(-600.0) && r0[2]>(100.0) && r0[2]<(1200.0))
      outputStroke += "vl-fh, ";
    //vl-bk
    if(((r0[0]>(-1500.0) && r0[0]<(-100.0)) || (r1[0]>(-1500.0) && r1[0]<(-100.0))) && r0[1]>(-500.0) && r0[1]<(500.0) && r0[2]>(100.0) && r0[2]<(1200.0))
      outputStroke += "vl-bk, ";
  }
  if (outputStroke.length() < 7)
    outputStroke += "unk"; //unknown
  else
    outputStroke.remove(outputStroke.length()-2);
}
