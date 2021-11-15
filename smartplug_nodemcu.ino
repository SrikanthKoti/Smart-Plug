/*
            This code uses EmonLib to calculate the power consumption.
            Also from this code we send raw input data to analog read
            of ct sensor.
*/
#include <FS.h>    
//for to connect to wifi.
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>          
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define RELAY_PIN D0
#define HOTSPOT_PIN D4

WiFiClient upload_http_client;      //sending data to website.

const int BURDEN_RESISTOR = 200;
double current, power;
float error = 0.03;

//same server for both getting and setting data.
const char* http_server = "sriki007.pythonanywhere.com";

//for getting values.
HTTPClient http;  
String s_payload;
String o_f_payload;

int count=1;
WiFiManager wifiManager;

int POWERCOUNT=1;
int HOTSPOTCOUNT;

void wifi()
{
  
  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //wifiManager.resetSettings();

  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("SmartPlug", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}
void wifireset()
{
//exit after config instead of connecting
  //wifiManager.setBreakAfterConfig(true);

  wifiManager.resetSettings();
  delay(500);
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration

    ESP.reset();
  

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

 // Serial.println("local ip");
//  Serial.println(WiFi.localIP());
  
}

double calculateCurrent(double raw_value){
  double raw_voltage = 3.3 * (raw_value / 1023);
  current = (1000 * (raw_voltage / BURDEN_RESISTOR) - error) / sqrt(2);
  power = current * 230;
  return current;
}

void upload_http_connect()
{    
    //for http server.
    if (upload_http_client.connect(http_server,80))
     {      
     upload_http_client.print(String("GET /upload_pc_data?power="+String(power/100.0)+"&"+"current="+String(current/100.0)) + " HTTP/1.1\r\n" +"Host: " + http_server + "\r\n" + "Connection: close\r\n\r\n");
     delay(3000);
     upload_http_client.print(String("GET /testing?irms="+String(current)+"&"+"analog="+String(current)+"&"+"analog2="+String(current)) + " HTTP/1.1\r\n" +"Host: " + http_server + "\r\n" + "Connection: close\r\n\r\n");
     delay(1000);
     Serial.println(String(power));     
     Serial.println(current);
     }
     else
     {
      Serial.println("There was a problem in connecting to server... pls check once.");
     }
     upload_http_client.stop();
     delay(10000);//for data uploading.
} 

String on_off_http_client()
{ Serial.println("at start of onoff fun");
   http.begin("http://sriki007.pythonanywhere.com/getonoffvalue");
    delay(200);
      int httpCode = http.GET();
      Serial.println(httpCode);
   o_f_payload = http.getString();
   delay(100);
   http.end();
   Serial.println("at onoff");
   Serial.println("bbb"+String(o_f_payload)+"bbb");
   return o_f_payload;
}

double readRawValue() {
  double raw_value = 0;
  double max_raw_value = 0;
  for(int i=0; i < 190; i++){
    raw_value = analogRead(A0);
    if(raw_value > max_raw_value){
      max_raw_value = raw_value;
    }
    yield();
  }
  Serial.println("max raw value : "+String(max_raw_value));
  return max_raw_value;
}

String schecdule_http_client()
{      Serial.println("at start schedule fun");
      http.begin("http://sriki007.pythonanywhere.com/getschedulevalue");
      delay(200);
      int httpCode = http.GET();
      Serial.println(httpCode);
      s_payload = http.getString();
      delay(100);
      http.end();
      Serial.println("at schedule");
      Serial.println("aaaa"+String(s_payload)+"aaaa");
      return s_payload;
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(HOTSPOT_PIN,INPUT);
  wifi();
}

void loop()
{ 
  bool HOTSPOT=digitalRead(HOTSPOT_PIN);
     if(digitalRead(HOTSPOT_PIN)==LOW)
    {
      Serial.println(digitalRead(HOTSPOT_PIN));
      Serial.println("Hotspot reset");
       wifireset();
    }
     if(strlen(WiFi.localIP().toString().c_str())==0)
    {
      Serial.println("Jarvis reconnect");
      wifi();
    }
        if(on_off_http_client()=="1" && schecdule_http_client()=="1")
      { Serial.println("in upload");
      double raw_value = readRawValue();
        calculateCurrent(raw_value);
        upload_http_connect();
        Serial.println("great");
        Serial.println("upload done");
        digitalWrite(RELAY_PIN,0);
      }
      else
      { Serial.println("in else");
        digitalWrite(RELAY_PIN,1);
  
}
}
