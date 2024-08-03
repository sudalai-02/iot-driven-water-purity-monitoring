float slopeprobe=-5.70;//calculated for temperature at 25C
float intercept=21.34;
#include "ESP8266_AT.h"
#define DOMAIN "api.thingspeak.com"
#define PORT "80"
#define API_WRITE_KEY "L5R1AZZ315DE1AKI"
#define CHANNEL_ID "963297"
#define SSID "Eswar"
#define PASSWORD "87654321"
char _buffer[400];
char _buffer2[400];
uint8_t Connect_Status;
uint8_t Sample = 0;
float voltageValue[30];
float voltageValue2;
void setup() {
 Serial.begin(115200);
 while(!ESP8266_Begin());
 ESP8266_WIFIMode(BOTH_STATION_AND_ACCESPOINT);/* 3 = Both (AP and 
STA) */
 ESP8266_ConnectionMode(SINGLE); /* 0 = Single; 1 = Multi */
 ESP8266_ApplicationMode(NORMAL); /* 0 = Normal Mode; 1 = Transperant Mode 
*/
 if(ESP8266_connected() == ESP8266_NOT_CONNECTED_TO_AP) /*Check WIFI 
connection*/
 ESP8266_JoinAccessPoint(SSID, PASSWORD); /*Connect to WIFI*/
 ESP8266_Start(0, DOMAIN, PORT); 
}
void loop() {//add minimum point to print voltage 

 Connect_Status = ESP8266_connected();
 if(Connect_Status == ESP8266_NOT_CONNECTED_TO_AP) /*Again check 
connection to WIFI*/
 ESP8266_JoinAccessPoint(SSID, PASSWORD); /*Connect to WIFI*/
 if(Connect_Status == ESP8266_TRANSMISSION_DISCONNECTED)
 ESP8266_Start(0, DOMAIN, PORT); /*Connect to TCP port*/
 //part1=(int)
 senddata();
}
double Phdata()
{
 int Navg =20 ;
 uint16_t datapH = 0;
 for(int i=0; i<Navg; i++){ //take 10 readings and then average them
 datapH += analogRead(A0);;
 }
 datapH = datapH/Navg;
 datapH = (datapH*(5.0/1023.0)-0.88)/3.52*14.00;
 if (datapH <0){
 datapH = 0;}
 return datapH;
 
}
double Turbidity()
{
 int Navg =20 ;
 int dataturbidity = 0;
 for(int i=0; i<Navg; i++){ //take 10 readings and then average them
 dataturbidity += analogRead(A1);
 }
 dataturbidity = dataturbidity/Navg;
 Serial.println(dataturbidity);
 dataturbidity = (dataturbidity*-5.69) + 4000;

 if (dataturbidity <0) 
 {dataturbidity = 0;
 }
 return dataturbidity;
}
void senddata()
{
 memset(_buffer, 0, 150);
 sprintf(_buffer, "GET /update?api_key=%s&field1=%d.%0.2d&field2=%d.%0.2d", 
API_WRITE_KEY, 
(int)Phdata(),(int)(Phdata()*100)%100,(int)Turbidity(),(int)(Turbidity()*100)%100); 
/*connect to thingspeak server to post data using your API_WRITE_KEY*/
 ESP8266_Send(_buffer);
 memset(_buffer2,0, 150);
 sprintf(_buffer2, "GET /update?api_key=%s&field2=%d.%0.2d", 
API_WRITE_KEY, (int)Turbidity(),(int)(Turbidity()*100)%100); /*connect to 
thingspeak server to post data using your API_WRITE_KEY*/
 ESP8266_Send(_buffer2);
 delay(1000); /* Thingspeak server delay */
}
/*
* ESP8266 AT
* http://www.electronicwings.com
*/ 
#include "ESP8266_AT.h"
int8_t Response_Status;
uint32_t TimeOut = 0;
char RESPONSE_BUFFER[DEFAULT_BUFFER_SIZE];
void Read_Response(char* _Expected_Response)
{
uint8_t ch, EXPECTED_RESPONSE_LENGTH = strlen(_Expected_Response);
uint32_t TimeCount = millis();
uint16_t Buffindex=0;
char RECEIVED_CRLF_BUF[EXPECTED_RESPONSE_LENGTH];

while(1)
{
if((DEFAULT_TIMEOUT+TimeOut) <= (millis()-TimeCount))
{
TimeOut = 0;
Response_Status = ESP8266_RESPONSE_TIMEOUT;
return;
}
if(Response_Status == ESP8266_RESPONSE_STARTING)
{
Response_Status = ESP8266_RESPONSE_WAITING;
}
while(Serial.available()> 0)
{
ch = Serial.read();
RESPONSE_BUFFER[Buffindex++] = ch;
memmove(RECEIVED_CRLF_BUF, RECEIVED_CRLF_BUF + 1, 
EXPECTED_RESPONSE_LENGTH-1);
RECEIVED_CRLF_BUF[EXPECTED_RESPONSE_LENGTH-1] = ch;
if(!strncmp(RECEIVED_CRLF_BUF, _Expected_Response, 
EXPECTED_RESPONSE_LENGTH))
{
TimeOut = 0;
Response_Status = ESP8266_RESPONSE_FINISHED;
return;
}
}
}
}
void ESP8266_Clear()
{
while(Serial.available()>0) Serial.read();
memset(RESPONSE_BUFFER,0,DEFAULT_BUFFER_SIZE);

}
void Start_Read_Response(char* _ExpectedResponse)
{
Response_Status = ESP8266_RESPONSE_STARTING;
do {
Read_Response(_ExpectedResponse);
} while(Response_Status == ESP8266_RESPONSE_WAITING);
}
void GetResponseBody(char* Response, uint16_t ResponseLength)
{
uint16_t i = 12;
char buffer[5];
while(Response[i] != '\r')
++i;
strncpy(buffer, Response + 12, (i - 12));
ResponseLength = atoi(buffer);
i += 2;
uint16_t tmp = strlen(Response) - i;
memcpy(Response, Response + i, tmp);
if(!strncmp(Response + tmp - 6, "\r\nOK\r\n", 6))
memset(Response + tmp - 6, 0, i + 6);
}
bool WaitForExpectedResponse(char* ExpectedResponse)
{
Start_Read_Response(ExpectedResponse); /* First read response */
if((Response_Status != ESP8266_RESPONSE_TIMEOUT))
return true; /* Return true for success */
return false; /* Else return false */

}
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse)
{
ESP8266_Clear();
Serial.println(ATCommand); /* Send AT command to ESP8266 */
return WaitForExpectedResponse(ExpectedResponse);
}
bool ESP8266_ApplicationMode(uint8_t Mode)
{
char _atCommand[20];
memset(_atCommand, 0, 20);
sprintf(_atCommand, "AT+CIPMODE=%d", Mode);
_atCommand[19] = 0;
return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}
bool ESP8266_ConnectionMode(uint8_t Mode)
{
char _atCommand[20];
memset(_atCommand, 0, 20);
sprintf(_atCommand, "AT+CIPMUX=%d", Mode);
_atCommand[19] = 0;
return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}
bool ESP8266_Begin()
{
for (uint8_t i=0;i<5;i++)
{
if(SendATandExpectResponse("ATE0","\r\nOK\r\n")||SendATandExpectResponse("
AT","\r\nOK\r\n"))
return true;
}

return false;
}
bool ESP8266_Close()
{
return SendATandExpectResponse("AT+CIPCLOSE=1", "\r\nOK\r\n");
}
bool ESP8266_WIFIMode(uint8_t _mode)
{
char _atCommand[20];
memset(_atCommand, 0, 20);
sprintf(_atCommand, "AT+CWMODE=%d", _mode);
_atCommand[19] = 0;
return SendATandExpectResponse(_atCommand, "\r\nOK\r\n");
}
uint8_t ESP8266_JoinAccessPoint(char* _SSID, char* _PASSWORD)
{
char _atCommand[60];
memset(_atCommand, 0, 60);
sprintf(_atCommand, "AT+CWJAP=\"%s\",\"%s\"", _SSID, _PASSWORD);
_atCommand[59] = 0;
if(SendATandExpectResponse(_atCommand, "\r\nWIFI CONNECTED\r\n"))
return ESP8266_WIFI_CONNECTED;
else{
if(strstr(RESPONSE_BUFFER, "+CWJAP:1"))
return ESP8266_CONNECTION_TIMEOUT;
else if(strstr(RESPONSE_BUFFER, "+CWJAP:2"))
return ESP8266_WRONG_PASSWORD;
else if(strstr(RESPONSE_BUFFER, "+CWJAP:3"))
return ESP8266_NOT_FOUND_TARGET_AP;
else if(strstr(RESPONSE_BUFFER, "+CWJAP:4"))
return ESP8266_CONNECTION_FAILED;
else
return ESP8266_JOIN_UNKNOWN_ERROR;

}
}
uint8_t ESP8266_connected()
{
SendATandExpectResponse("AT+CIPSTATUS", "\r\nOK\r\n");
if(strstr(RESPONSE_BUFFER, "STATUS:2"))
return ESP8266_CONNECTED_TO_AP;
else if(strstr(RESPONSE_BUFFER, "STATUS:3"))
return ESP8266_CREATED_TRANSMISSION;
else if(strstr(RESPONSE_BUFFER, "STATUS:4"))
return ESP8266_TRANSMISSION_DISCONNECTED;
else if(strstr(RESPONSE_BUFFER, "STATUS:5"))
return ESP8266_NOT_CONNECTED_TO_AP;
else
return ESP8266_CONNECT_UNKNOWN_ERROR;
}
uint8_t ESP8266_Start(uint8_t _ConnectionNumber, char* Domain, char* Port)
{
bool _startResponse;
char _atCommand[60];
memset(_atCommand, 0, 60);
_atCommand[59] = 0;
if(SendATandExpectResponse("AT+CIPMUX?", "CIPMUX:0"))
sprintf(_atCommand, "AT+CIPSTART=\"TCP\",\"%s\",%s", Domain, Port);
else
sprintf(_atCommand, "AT+CIPSTART=\"%d\",\"TCP\",\"%s\",%s", 
_ConnectionNumber, Domain, Port);
_startResponse = SendATandExpectResponse(_atCommand, "CONNECT\r\n");
if(!_startResponse)
{
if(Response_Status == ESP8266_RESPONSE_TIMEOUT)
return ESP8266_RESPONSE_TIMEOUT;

return ESP8266_RESPONSE_ERROR;
}
return ESP8266_RESPONSE_FINISHED;
}
uint8_t ESP8266_Send(char* Data)
{
char _atCommand[20];
memset(_atCommand, 0, 20);
sprintf(_atCommand, "AT+CIPSEND=%d", (strlen(Data)+2));
_atCommand[19] = 0;
SendATandExpectResponse(_atCommand, "\r\nOK\r\n>");
if(!SendATandExpectResponse(Data, "\r\nSEND OK\r\n"))
{
if(Response_Status == ESP8266_RESPONSE_TIMEOUT)
return ESP8266_RESPONSE_TIMEOUT;
return ESP8266_RESPONSE_ERROR;
}
return ESP8266_RESPONSE_FINISHED;
}
uint16_t Read_Data(char* _buffer)
{
uint16_t len = 0;
delay(100);
while(Serial.available() > 0)
_buffer[len++] = Serial.read();
return len;
}
#ifndef ESP8266_AT_H_

#define ESP8266_AT_H_
#include <Arduino.h> /* Include Arduino library */
#include <SoftwareSerial.h>
#define SREG _SFR_IO8(0x3F)
#define DEFAULT_BUFFER_SIZE 160
#define DEFAULT_TIMEOUT 10000
/* Connection Mode */
#define SINGLE 0
#define MULTIPLE 1
/* Application Mode */
#define NORMAL 0
#define TRANSPERANT 1
/* Application Mode */
#define STATION 1
#define ACCESSPOINT 2
#define BOTH_STATION_AND_ACCESPOINT 3
enum ESP8266_RESPONSE_STATUS{
 ESP8266_RESPONSE_WAITING,
 ESP8266_RESPONSE_FINISHED,
 ESP8266_RESPONSE_TIMEOUT,
 ESP8266_RESPONSE_BUFFER_FULL,
 ESP8266_RESPONSE_STARTING,
 ESP8266_RESPONSE_ERROR
};
enum ESP8266_CONNECT_STATUS {

 ESP8266_CONNECTED_TO_AP,
 ESP8266_CREATED_TRANSMISSION,
 ESP8266_TRANSMISSION_DISCONNECTED,
 ESP8266_NOT_CONNECTED_TO_AP,
 ESP8266_CONNECT_UNKNOWN_ERROR
};
enum ESP8266_JOINAP_STATUS {
 ESP8266_WIFI_CONNECTED,
 ESP8266_CONNECTION_TIMEOUT,
 ESP8266_WRONG_PASSWORD,
 ESP8266_NOT_FOUND_TARGET_AP,
 ESP8266_CONNECTION_FAILED,
 ESP8266_JOIN_UNKNOWN_ERROR
};
void Read_Response(char* _Expected_Response);
void ESP8266_Clear();
void Start_Read_Response(char* _ExpectedResponse);
void GetResponseBody(char* Response, uint16_t ResponseLength);
bool WaitForExpectedResponse(char* ExpectedResponse);
bool SendATandExpectResponse(char* ATCommand, char* ExpectedResponse);
bool ESP8266_ApplicationMode(uint8_t Mode);
bool ESP8266_ConnectionMode(uint8_t Mode);
bool ESP8266_Begin();
bool ESP8266_Close();
bool ESP8266_WIFIMode(uint8_t _mode);
uint8_t ESP8266_JoinAccessPoint(char* _SSID, char* _PASSWORD);
uint8_t ESP8266_connected();
uint8_t ESP8266_Start(uint8_t _ConnectionNumber, char* Domain, char* Port);
uint8_t ESP8266_Send(char* Data);
uint16_t Read_Data(char* _buffer);
#endif /* ESP8266_AT_H_ *
