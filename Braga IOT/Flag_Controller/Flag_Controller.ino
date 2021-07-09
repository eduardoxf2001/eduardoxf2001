#include <ESP8266WiFi.h>                                                              //Needed for starting WiFi cnd configure it
#include <WiFiUDP.h>                                                                  //Needed for UDP protocol
#include <string.h>                                                                   //Needed to get string length                     
#include <stdlib.h>                                                                   //Needed for Converting String to INT

#define coil1 14                                                                      //Define what IO pins are connected to the coils of relays
#define coil2 4


unsigned int UDPPort = 2390;                                                          //UDP PORT
WiFiUDP Udp;                                                                          //Create instance of WiFiUDP

IPAddress local_IP(192,168,4,22);                                                     //Access Point IP
IPAddress gateway(192,168,4,22);                                                      //Access Point is itselft the gateway
IPAddress subnet(255,255,255,0);                                                      //IP Mask

int flag = 0;                                                                         //Beach's Flag State (1-Green|2-Yellow|3-Red)
float temp = 0;                                                                       //Temperature
int flag_a = 1;                                                                       //Previous Beach's Flag state 



void setup() {
  Serial.begin(9600);                                                                 //Starting Serial Comunications with 9600 Baud Rate
  pinMode(coil1, OUTPUT);                                                             //Define coils as OUTPUTS
  pinMode(coil2, OUTPUT);
  
  WiFi.mode(WIFI_AP);                                                                 //Start WiFi mode as Access Point
  Serial.print("Configuring soft-AP ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!"); //Configure Access point with the defined IP,Gateway and Mask
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("ESP8266_AP") ? "Ready" : "Failed!");                    //Start Access Point with the ID Vodafone-FA81A2
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());                                                    //Prints out the IP of the WiFi access point

  Udp.begin(UDPPort);                                                                 //Start UDP comunications with the port defined
}


  
void loop() {
  int packetSize = Udp.parsePacket();                                                 //Preparing UDP reading and get size of the packet received
  char buff[5] = {0};                                                                 //Buffer that will receive the packet 
  
  if (packetSize){                                                                    //If there is a packet, store it in the Buffer
    Udp.read(buff, 5);
    Serial.print((String)"Buffer:"+buff+"\n");   
  }

  if (strlen(buff) == 1){                                                             //If buffer string length is 1,
    flag = atoi(buff);                                                                //Converts String to INT and stores it on flag
  }
  else if (strlen(buff) == 3){                                                        //If buffer string length is 3,
    temp = (float)atoi(buff)/10;                                                      //Converts String to Float divide it by 10 and stores it on temp
  }

  Serial.print((String)"Temp: "+temp+"C\n");
  Serial.print((String)"Previous Flag: "+flag_a+"\n");  
  Serial.print((String)"Flag: "+flag+"\n");   
  
  if ((flag - flag_a) == 1){                                                          //Compare current flag state with previous flag state, if it needs to get to next flag motor, goes forward
    Serial.print("Forward\n");
    digitalWrite(coil1, 1);
    delay(1000);                                                                      //Time for the motor to move

  }
  else if((flag - flag_a) == -1){                                                     //Compare current flag state with previous flag state, if it needs to get to previous flag, motor goes backwards
    Serial.print("Backwards\n");
    digitalWrite(coil2, 1);
    delay(1000);                                                                      //Time for the motor to move

  }                                                                                   //Special cases for GREEN to RED and RED to Green
  else if ((flag - flag_a)== 2){                                                      //Compare current flag state with previous flag state, if it's on GREEN and it need's to be on RED, motor goes backwards
    Serial.print("Backwards\n");
    digitalWrite(coil2, 1);
    delay(1000);                                                                      //Time for the motor to move                               

  }
  else if ((flag - flag_a)== -2){                                                     //Compare current flag state with previous flag state, if it's on RED and it need's to be on GREEN, motor goes forward
    Serial.print("Forward\n");
    digitalWrite(coil1, 1);
    delay(1000);                                                                      //Time for the motor to move
        
  }                                                                      
  digitalWrite(coil1, 0);                                                             //Stop de motor
  digitalWrite(coil2, 0); 
  flag_a = flag;                                                                      //After all it's done declare previous flag state equals to current flag state
  Serial.print("\n");         
}
