#include <SPI.h>
#include <SD.h>
const int chipSelect = 10;

#include <Wire.h> 
#include <Sodaq_DS3231.h>
DateTime now;

long now_data[16];
long before_data[16];
long sum_data[16];
int maximum;
long max_push;
int now_ave, before_ave;
float now_light_percent;
long now_sum;
int cloud,c,cloudy_time;

//Mux control pins
int s0 = 2;
int s1 = 3;
int s2 = 4;
int s3 = 5;

int value ;
void setup(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
  
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();
  DateTime dt(2021, 6, 7, 0, 00, 0, 0);
  rtc.setDateTime(dt);
  while (!Serial) {
    ; 
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");

  if (SD.exists("datalog.txt"))
  {
    SD.remove("datalog.txt");
  }
}


void loop(){
  
  now = rtc.now();
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
      dataFile.print(now.hour());
      dataFile.print(':');
      dataFile.print(now.minute());
      dataFile.print(':');
      dataFile.print(now.second());
      Serial.print(now.hour());
      Serial.print(':');
      Serial.print(now.minute());
      Serial.print(':');
      Serial.print(now.second());}
      
  
  for(int i = 0; i < 16; i ++){
    value = readMux(i);
    now_data[i]= value;
  
    dataFile.print(",");
    dataFile.print(value);
    // print to the serial port too:
    Serial.print(",");
    Serial.print(value);  
    delay(500);
   }

   maximum = find_max(now_data,0);
   find_min(now_data,0);
   max_push = max_push + maximum;
   for(int j=0; j < 16; j++){
    sum_data[j] += now_data[j];
    }
   find_max(sum_data,1);
   find_min(sum_data,1);
   Serial.println("");
   Serial.print("max_push: ");
   Serial.println(max_push);
   now_sum=0;
   for(int k=0;k<16;k++){
    now_sum += now_data[k];
    }
   now_ave = now_sum/16;
   now_light_percent = (now_ave/890)*100;
   Serial.print("now_light_percent : ");
   Serial.print(now_light_percent);
   Serial.println("%");
   if(c==1){
    if(now_ave - before_ave > 40){
      c=0;
    }
    else {cloudy_time++;}
   }
   if(c==0){
    if(before_ave - now_ave > 40){
      cloud++;
      c=1;
      cloudy_time++;
    }
   }
   Serial.print("number_of_clouds : ");
   Serial.println(cloud);
   Serial.print("cloudy_time : ");
   Serial.print(cloudy_time*2);
   Serial.println(" minutes");
   weather_condition(c,now_light_percent);
   Serial.println("--------------------------------------------------");

   for(int m=0;m<16;m++){
    before_data[m] = now_data[m];
    }
    before_ave = now_ave;
   
   dataFile.println("");
   dataFile.close();
   delay(2*60000);
}


//functions

float readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};
  int muxChannel[16][4]={
    {1,1,1,0}, //sensor 0
    {0,0,0,0}, //sensor 1
    {1,0,0,0}, //sensor 2
    {0,1,0,0}, //sensor 3
    {1,1,0,0}, //sensor 4
    {0,0,1,1}, //sensor 5
    {1,0,1,1}, //sensor 6
    {0,1,1,1}, //sensor 7
    {1,1,0,1}, //sensor 8
    {1,1,1,1}, //sensor 9
    {0,1,0,1}, //sensor 10
    {1,0,0,1}, //sensor 11
    {0,0,0,1}, //sensor 12
    {0,1,1,0}, //sensor 13
    {1,0,1,0}, //sensor 14
    {0,0,1,0}  //sensor 15
  };

  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
  delay(500);
  int val = analogRead(A0);
  return val;
}

long find_max(long a[],int z){
  Serial.println("");
  long x=0;
  for(int i=0;i<16;i++){
    if(a[i]>x){
      x=a[i];
    }
  }
  if(z==0){
    Serial.print("now_max: ");
    return x;
    }
  if(z==1){
    Serial.print("sum_max: ");    
    }
  Serial.print(x);
  printf(" --> ");
  Serial.print("sensor number: ");
  for(int j =0;j<16;j++){
    if(a[j]==x){
      Serial.print(j+1);
      Serial.print(",");
    }
  }
}

long find_min(long a[],int z){
  Serial.println("");
  long x=100000;
  for(int i=0;i<16;i++){
    if(a[i]<x){
      x=a[i];
    }
  }
  if(z==0){
    Serial.print("now_min: ");
    }
  if(z==1){
    Serial.print("sum_min: ");    
    }
  Serial.print(x);
  printf(" --> ");
  Serial.print("sensor number: ");
  for(int j =0;j<16;j++){
    if(a[j]==x){
      Serial.print(j+1);
      Serial.print(",");
    }
  }
}

void weather_condition(int x, float y){
  if(x==0){
    Serial.print("The sky is clear, ");
    }
  if(x==1){
    Serial.print("The sky is cloudy, ");
    }
  if(y>85){
    Serial.println("and the sun is shining brutally.");
    }
  if(y>65 && y<85){
    Serial.println("and the sun is shining softly.");
    }
  if(y>35 && y<65){
    Serial.println("and the sun is shining weakly.");
    }
  if(y<35){
    Serial.println("and the sun is off.");
    }
  }
