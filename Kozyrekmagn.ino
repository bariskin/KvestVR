//*******************************************
// флаги включения устройств 
int flag_output_off_on_1 = 0;
int flag_output_off_on_2 = 0;
//*******************************************


#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
/*
10.0.0.x
1- router
8- ноутбук1
9- ноутбук2
11 - труба аналоговый вход  
12 - рубльник аналоговый вход
13 - вибро
14 - решетка
15 -  kozyrekmagn,
16 - козырек2
17 - козырек1
18 0 ручка аналоговый вход

*/

const char* ssid = "DLink6610";                     // WiFi APN Name
const char* password = "1234qwerasdf";              // WiFi passphrase


unsigned char ip_sh1[] = {10,0,0,8};                // Server_1 IP
unsigned char ip_sh2[] = {10,0,0,9};                // Server_2 IP
unsigned char ip_sh3[] = {10,0,0,3};                // Server_3 


IPAddress ip(10,0,0,15);                            //  kozyrekmagn
IPAddress gateway(10,0,0,1);
IPAddress subnet(255,255,255,0);

unsigned int port = 6000;                       // Server port (listen)

int pin_out_LED = 2;                            // LED (internal_LED=2)
int pin_out_1 = 5;
int pin_out_2 = 4;
int pin_in_1 = 12;                              // Button (pin D0=16, D1=5, D2=4, D3=0, D4=2, D5=14, D6=12, D7=13, D8=15, D9=3, D10=1, D11=9, D21=10)
int pin_in_2 = 14;

byte TRIGGER1_flag = 0;
byte TRIGGER2_flag = 0;
byte input_1_flag = 1;
byte input_2_flag = 1;


WiFiUDP Udp;

unsigned int localUdpPort = 8888;                // Arduino port (listen)
char incomingPacket[255];                        // Arduino incoming buffer size
char replyPacket[] = "Board status: received: "; // 

void setup()
{ 
  pinMode(pin_out_1, OUTPUT);
  pinMode(pin_out_2, OUTPUT);
  pinMode(pin_in_1,  INPUT_PULLUP);                 // подтяжка пина  к плюсу 
  pinMode(pin_in_2,  INPUT_PULLUP);                 // подтяжка пина  к плюсу
  pinMode(pin_out_LED, OUTPUT);
 
  WiFi.config(ip, gateway, subnet);
  Serial.begin(115200);
  
  Serial.printf("Board status: Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(pin_out_LED,HIGH); 
    delay(50);
    digitalWrite(pin_out_LED,LOW); 
    delay(400);
    Serial.print(".");
  }
  Serial.printf("Board status: Connected to %s", ssid);
  Udp.begin(localUdpPort);
  Serial.printf("Board status: listening at IP: %s, UDP port: %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

// ----------------------------------MAIN PART -------------------------------------- //
void loop()
{
 int packetSize = Udp.parsePacket();
 if (packetSize)
 {
   digitalWrite(pin_out_LED,HIGH);
      // ------------------------------------ LISTEN INBOUND ----------------------------------- //
   Serial.printf("Board status: Received %d bytes from %s, port: %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
   int len = Udp.read(incomingPacket, 255);
   if (len > 0) 
   {
    incomingPacket[len] = 0;
   }
   if (incomingPacket[0] == '#' && incomingPacket[1] == 'P' && incomingPacket[2] == 'I' && incomingPacket[3] == 'N') 
   { 
    // --------------------------------------- DIGITAL OUTPUT ----------------------------- //
    if (incomingPacket[5] == '1') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { input_1_flag = 0; Serial.print("Board status: PIN_1 = DISABLED\r\n"); }
     if (incomingPacket[7] == '0') { digitalWrite(pin_out_1,LOW); Serial.print("Board status: PIN_1 = OFF\r\n"); }
     if (incomingPacket[7] == '1') { digitalWrite(pin_out_1,HIGH); Serial.print("Board status: PIN_1 = ON\r\n"); }
    }
    if (incomingPacket[5] == '2') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { input_2_flag = 0; Serial.print("Board status: PIN_2 = DISABLED\r\n"); }
     if (incomingPacket[7] == '0') { digitalWrite(pin_out_2,LOW); Serial.print("Board status: PIN_2 = OFF\r\n"); }
     if (incomingPacket[7] == '1') { digitalWrite(pin_out_2,HIGH); Serial.print("Board status: PIN_2 = ON\r\n"); }
    }
    if ((incomingPacket[5] == 'A') && (incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) 
    { 
   //   input_analog_flag = 0; Serial.print("Board status: PIN_A = DISABLED\r\n"); 
    }
    // ----------------------------------- ANSWER TO SERVER ----------------------------------- //
   }
   
   Serial.printf("UDP packet contents: %s\n", incomingPacket);
   Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
   Udp.write(replyPacket); 
   Udp.write(incomingPacket); 
   Udp.endPacket();
   if ((TRIGGER1_flag == 0) && (TRIGGER2_flag == 0)) { digitalWrite(pin_out_LED,LOW); }
 }
 else 
 {
   digitalWrite(pin_out_LED,LOW);
 } 
 

 // --------------------------------------- DIGITAL INPUT ----------------------------- //
 if ((input_1_flag == 1) && (TRIGGER1_flag == 0) && (digitalRead(pin_in_1) == HIGH))
 {
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER1_ON"); 
  Udp.endPacket();

  Udp.beginPacket(ip_sh2, port); 
  Udp.write("#TRIGGER1_ON"); 
  Udp.endPacket();

  Udp.beginPacket(ip_sh3, port); 
  Udp.write("#TRIGGER1_ON"); 
  Udp.endPacket();
  
  TRIGGER1_flag = 1;
  Serial.printf("#TRIGGER1_ON");
 }

 if ((input_1_flag == 1) && (TRIGGER1_flag == 1) && (digitalRead(pin_in_1) == LOW))
 {
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER1_OFF"); 
  Udp.endPacket();
  Udp.beginPacket(ip_sh2, port); 
  Udp.write("#TRIGGER1_OFF"); 
  Udp.endPacket();
  Udp.beginPacket(ip_sh3, port); 
  Udp.write("#TRIGGER1_OFF"); 
  Udp.endPacket();
  TRIGGER1_flag = 0;
  Serial.printf("#TRIGGER1_OFF");
 }

 if ((input_2_flag == 1) && (TRIGGER2_flag == 0) && (digitalRead(pin_in_2) == HIGH))
 {
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER2_ON"); 
  Serial.printf("#TRIGGER2_ON");
  Udp.endPacket();
  Udp.beginPacket(ip_sh2, port); 
  Udp.write("#TRIGGER2_ON"); 
  Udp.endPacket();
  Udp.beginPacket(ip_sh3, port); 
  Udp.write("#TRIGGER2_ON"); 
  Udp.endPacket();
  TRIGGER2_flag = 1;
 }

 if ((input_2_flag == 1) && (TRIGGER2_flag == 1) && (digitalRead(pin_in_2) == LOW))
 {
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER2_OFF"); 
  Serial.printf("#TRIGGER2_OFF");
  Udp.endPacket();
  Udp.beginPacket(ip_sh2, port); 
  Udp.write("#TRIGGER2_OFF"); 
  Udp.endPacket();
  Udp.beginPacket(ip_sh3, port); 
  Udp.write("#TRIGGER2_OFF"); 
  Udp.endPacket();
  TRIGGER2_flag = 0;
 }

digitalWrite(pin_out_LED,HIGH); 
delay(50);
digitalWrite(pin_out_LED,LOW); 
delay(150);

 
}
