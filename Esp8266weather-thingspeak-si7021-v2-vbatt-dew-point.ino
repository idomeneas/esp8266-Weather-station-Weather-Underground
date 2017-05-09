#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_BMP085.h> 

#include "SparkFun_Si7021_Breakout_Library.h"   
  
const char* ssid     = "idomeneas"; 
const char* password = "H456JV4J5T45-6643";
const int sleepTimeS = 600; //18000 for Half hour, 300 for 5 minutes etc.
                               
///////////////Weather//////////////////////// 
String apiKey = "2XEIHQVX5HD9LG12";
const char* server2 = "api.thingspeak.com";

const char* host = "weatherstation.wunderground.com";  
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
const char* WUID    = "ICHANIA8";
const char* WUPASS   = "torpilakatos";
float vbatt= 0;



Weather sensor;

Adafruit_BMP085 bmp;
               
           


void setup()
{
  Serial.begin(115200);
  
   bmp.begin();
   sensor.begin();
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void loop(){ 
   // Measure Relative Humidity from the HTU21D or Si7021
  float humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  float tempf = sensor.getTempF();
  float tempc = (tempf-32)* 5/9;
  
  float p1 = (bmp.readPressure()/100);
  float p2 = (p1 / pow((1.0 - ( 30 / 44330.0 )), 5.255));
  float p3 = (p2 *100);
  float baromin= (p3*0.0002953);
   
  float dewptc = (dewPointFast(tempc, sensor.getRH())); 
  float dewptf = ((dewptc*9)/5)+32;
  
    delay(10);
   //float dp = tempc - (((100-humidity)/5));
    delay(10);
    vbatt= (0.0041115630 * analogRead(A0));
  //check sensor data
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("tempF= ");
  Serial.print(tempf);
  Serial.println(" *F");
  Serial.print("tempC= ");
  Serial.print(tempc);
  Serial.println(" *C");
  Serial.print("dew pointC= ");
  Serial.println(dewptc);
  Serial.print("dew pointF= ");
  Serial.println(dewptf);
  Serial.print("humidity= ");
  Serial.println(humidity);
  Serial.print("baro= ");
  Serial.println(baromin);
  Serial.print("baro2= ");
  Serial.println(p1);
  Serial.print("baro3= ");
  Serial.println(p2);
  Serial.print("Vbatt= ");
  Serial.println(vbatt);
  
  //Send data to Weather Underground
   Serial.println("Send to WU Sensor Values");
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
   
    return;
  }


   // We now create a URI for the request
  String url = "/weatherstation/updateweatherstation.php?ID=";
  url += WUID;
  url += "&PASSWORD=";
  url += WUPASS;
  url += "&dateutc=now";
  url += "&tempf=";
  url += tempf;
  url += "&dewptf=";
  url += dewptf;
  url += "&humidity=";
  url += humidity;
  url += "&baromin=";
  url += baromin;
  url += "&softwaretype=Arduino-ESP8266&action=updateraw";
    
    Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
    
    if (client.connect(server2,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(tempc);
           postStr +="&field2=";
           postStr += String(humidity);
           postStr +="&field3=";
           postStr += String(p2);
            postStr +="&field4=";
           postStr += String(vbatt);
           
            postStr +="&field5=";
           postStr += String(dewptc);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n"); 
     client.print("Host: api.thingspeak.com\n"); 
     client.print("Connection: close\n"); 
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     client.print("Content-Type: application/x-www-form-urlencoded\n"); 
     client.print("Content-Length: "); 
     client.print(postStr.length()); 
     client.print("\n\n"); 
     client.print(postStr);
           
 
         
  }
  client.stop();
   delay(2500); 
    sleepMode();    

}


double dewPointFast(double celsius, double humidity)
{
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
        double Td = (b * temp) / (a - temp);
        return Td;
}


  WiFiClient client;
 

void sleepMode(){
  Serial.print(F("Sleeping..."));
  ESP.deepSleep(sleepTimeS * 1000000);
}

