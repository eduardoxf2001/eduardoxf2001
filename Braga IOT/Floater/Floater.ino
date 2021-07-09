#include <Wire.h>                                                                               //Necessary for starting I2C communications
#include <ESP8266WiFi.h>                                                                        //Necessary for WiFi communications
#include <WiFiUDP.h>                                                                            //Necessary for UDP protocol
#include "SparkFun_MMA8452Q.h"                                                                  //Necessary for getting date from Accelerometer 
#include <stdlib.h>                                                                             //Necessary for converting INT for String

MMA8452Q accel;                                                                                 //Create the instance of the MMA8452Q(Accelerometer)
#define time_m_ripple 30                                                                        //Time that takes to calculate the average Ripple in seconds(longest time better accuracy)

const char* apIP = "192.168.4.22";                                                              //IP of the access point                                                      
unsigned int UDPPort = 2390;                                                                    //UDP port for communication UDP beetween devices

WiFiUDP Udp;                                                                                    //Create the instance of the WiFiUDP      

int count = 0;                                                                                  //Variable that counts time
float temp = 0;                                                                                 //that stores temperature
float maxi = -9;                                                                                //stores maximum read value of the accelerometer
float mini = 9;                                                                                 //stores minimum read value of the accelerometer
float ripple = 0;                                                                               //Stores the ripple value
float ripple_m = 0;                                                                             //Stores the ripple mean value



void setup() {
  Serial.begin(9600);                                                                           //Begin serial communications at 9600 Baud Rate
  Wire.begin();                                                                                 //Initialize I2C communication
  WiFi.begin("ESP8266_AP");                                                                     //Joins WiFi network with the ID "ESP8266_AP"
  
  while (WiFi.status() != WL_CONNECTED) {                                                       //Waits until device is succesfull connect to the network
    delay(500);
    Serial.print(".");
  }  
  Serial.println("\n WiFi connected");              
  Serial.println("IP address: " + WiFi.localIP().toString());                                   //Shows the IP of this device
  Udp.begin(UDPPort);                                                                           //Starts UDP protocol with the port Defined

  if (accel.begin() == false) {                                                                 //Waits until device connects successfully to the accelerometer
    Serial.println("Not Connected. Please check connections and read the hookup guide.");
    while (1);
  }
}



void send_flag(){                                                                               //This function depending on the average Ripple value, sends an 3, 2 or 1 to the other device           
  Serial.print((String)"Average Ripple="+ripple_m+"\n");                                        //3 - Red Flag
  if (ripple_m>2){                  //Red                                                       //2 - Yellow Flag
    Udp.beginPacket(apIP, UDPPort);                                                             //1 - Green Flag
    Udp.write("3"); 
    Udp.endPacket();
  }
  else if (ripple_m>1){             //Yellow
    Udp.beginPacket(apIP, UDPPort);
    Udp.write("2");
    Udp.endPacket();
  }
  else {                            //Green
    Udp.beginPacket(apIP, UDPPort);
    Udp.write("1"); 
    Udp.endPacket();
  }
}



void mean_ripple(){
  float Z = 0;
  if(count%10==0){                                                                              //Each 10 increments of count read Z axis of the accelerometer and registers the maximum and minimum value of the ripple
    Z = accel.getCalculatedZ();
    if (Z>maxi){
      maxi = Z;
    }
    else if (Z<mini){
      mini = Z;  
    }
  }
  
  if (count%100==0){                                                                            //Each 100 increments, calculates the difference between the maximum and the minimum and adds that value to the value inside ripple, it stores the result in ripple
    ripple += (maxi-mini);

    if (count==(time_m_ripple*10)){                                                             //Each every average ripple time *10 calulates the average ripple and resets count and ripple
      Serial.print((String)"\t The mean ripple is:"+(ripple/(time_m_ripple/10))+"\n");
      ripple_m = ripple/(time_m_ripple/10);
      count = 0;
      ripple = 0;
    }
      
    Serial.print((String)"\t Peak Values:"+(maxi-mini)+"\n");                           
    maxi = -9;                                                                                  //Reset maximum and minimum values
    mini = 9;
    }
  Serial.print((String)"\nCount="+count+"\n");
}

    

void loop() {
  char temp_str[5];                                                                              //This buffer will store the temperature value converted to string
  count++;
  temp = ((analogRead(A0)/17)*0.5)*10;                                                           //Formula that gives temperature times 10 based on adc current state
  itoa(temp,temp_str,10);                                                                        //Converts temperature INT value to string
  
  Udp.beginPacket(apIP, UDPPort);                                                                //Sends temperature in format string to the other device
  Udp.write(temp_str); 
  Udp.endPacket();
  
  Serial.print("Temperature:");
  Serial.print(temp/10);
  Serial.print("  ADC:");
  Serial.print(analogRead(A0));
  Serial.print("\n");

  if (accel.available()) {                                                                      //Shows on the serial monitor the values in real time of the accelarometer
    Serial.print(accel.getCalculatedX(), 3);                                                   
    Serial.print("\t");
    Serial.print(accel.getCalculatedY(), 3);
    Serial.print("\t");
    Serial.print(accel.getCalculatedZ(), 3);
    Serial.println();
  }

  mean_ripple();                                                                                //Calls the function that calulates the mean ripple
  send_flag();                                                                                  //Calls the function that sends the flag to the other device
  /*delay(100);*/                                                                               //For the sake of presentation purposes we will not use this delay, but for practical purposes it would be used
}
