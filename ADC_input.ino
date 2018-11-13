/*
10.0.0.x
1- router
8- ноутбук1
9- ноутбук2
11 - труба аналоговый вход  
12 - рубльник аналоговый вход
13 - вибро
14 - решетка
15 - козырек2
16 - козырек2
17 - козырек1
18 0 ручка аналоговый вход
*/

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

ADC_MODE(ADC_TOUT);

const char* ssid = "DLink6610";                     // WiFi APN Name
const char* password = "1234qwerasdf";              // WiFi passphrase


//const char* ssid = "RT-WiFi-BDB8";
//const char* password = "2731007085";

unsigned char ip_sh1[] = {10,0,0,8};                // Server_1 IP
unsigned char ip_sh2[] = {10,0,0,9};                // Server_2 IP
unsigned char ip_sh3[] = {10,0,0,3};                // Server_3 IP


IPAddress ip(10,0,0,12);                            //статический IP для рубильника
IPAddress gateway(10,0,0,1);
IPAddress subnet(255,255,255,0);


unsigned int port = 6000;                           // Server port (listen)

int pin_out_LED = 2;                                // LED (internal_LED=2)
int pin_out_1 = 5;
int pin_out_2 = 4;
int pin_in_1 = 12;                                  // Button (pin D0=16, D1=5, D2=4, D3=0, D4=2, D5=14, D6=12, D7=13, D8=15, D9=3, D10=1, D11=9, D21=10)
int pin_in_2 = 14; 
char vccstring[16];
double prev_vcc = 0;
byte TRIGGER1_flag = 0;
byte TRIGGER2_flag = 0;
byte input_1_flag = 1;
byte input_2_flag = 1;
byte input_analog_flag = 1;

float VCCinput = 0 ;
double VCCvalue = 0 ;

double Sumvcc;
double vcc;

WiFiUDP Udp;
unsigned int localUdpPort = 8888;                  // Arduino port (listen)
char incomingPacket[255];                          // Arduino incoming buffer size

char replyPacket[] = "Board status: received: ";   // 

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
void loop(){
  
  // if there's data available, read a packet 
    int packetSize = Udp.parsePacket();
 
 if (packetSize)
 {
   digitalWrite(pin_out_LED,LOW);
   // ------------------------------------ LISTEN INBOUND ----------------------------------- //
   Serial.printf("Board status: Received %d bytes from %s, port: %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
  
// read the packet into incomingPacket
   int len = Udp.read(incomingPacket, 255);
  
   if (len > 0) 
   {
    incomingPacket[len] = 0;
   }

   if (incomingPacket[0] == '#' && incomingPacket[1] == 'P' && incomingPacket[2] == 'I' && incomingPacket[3] == 'N') 
   { Serial.print("Board status: №#PIN");
    // --------------------------------------- DIGITAL OUTPUT ----------------------------- //
    if (incomingPacket[5] == '1') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { input_1_flag = 0; Serial.print("Board status: PIN_1 = DISABLED\r\n"); }
     if ((input_1_flag == 1) && (incomingPacket[7] == '0')) { digitalWrite(pin_out_1,LOW); Serial.print("Board status: PIN_1 = OFF"); }
     if ((input_1_flag == 1) && (incomingPacket[7] == '1')) { digitalWrite(pin_out_1,HIGH); Serial.print("Board status: PIN_1 = ON"); }
    }
    if (incomingPacket[5] == '2') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { input_2_flag = 0; Serial.print("Board status: PIN_2 = DISABLED\r\n"); }
     if ((input_2_flag == 1) && (incomingPacket[7] == '0')) { digitalWrite(pin_out_2,LOW); Serial.print("Board status: PIN_2 = OFF"); }
     if ((input_2_flag == 1) && (incomingPacket[7] == '1')) { digitalWrite(pin_out_2,HIGH); Serial.print("Board status: PIN_2 = ON"); }
    }
    if ((incomingPacket[5] == 'A') && (incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) 
    { 
      input_analog_flag = 0; Serial.print("Board status: PIN_A = DISABLED\r\n"); 
    }
    
   }
   
   // ----------------------------------- ANSWER TO SERVER ----------------------------------- //
   Serial.printf("UDP packet contents: %s\n", incomingPacket);
   Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  // Udp.write(replyPacket);
  // Udp.write(incomingPacket);
   Udp.write(vccstring,16);
   Udp.endPacket();
 }
 else 
  {
    digitalWrite(pin_out_LED,HIGH);
  }  
  
 // ----------------------------------- ANALOG VOLTAGE READ -------------------------------- //

     Sumvcc = 0;

     
     vcc =  (double) analogRead(A0)/1024*3.3;
     Sumvcc += vcc;
     
     vcc =  (double) analogRead(A0)/1024*3.3;
     Sumvcc += vcc;

     vcc =  (double) analogRead(A0)/1024*3.3;
     Sumvcc += vcc;
     
     Sumvcc /= 3;
      
     
 // ----------------------------------- ANALOG VOLTAGE PACKET SEND ------------------------- //
 Serial.println();
 Serial.println();
 Serial.print("Connect to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);             // Connecting to WiFi
 dtostrf(Sumvcc,16,2,vccstring);

  Serial.print("Current voltage Vcc: ");
  Serial.println(Sumvcc);
 

 Udp.beginPacket(ip_sh1,port);////////////////////////////////////////////////////////////////////
 Udp.write(vccstring,16);
 Udp.endPacket();////////////////////////////////////////////////////////////////////////////
 
 delay(1);
 
 Udp.beginPacket(ip_sh2,port);////////////////////////////////////////////////////////////////////
 Udp.write(vccstring,16);
 Udp.endPacket();
 
  delay(1);
  
 Udp.beginPacket(ip_sh3,port);////////////////////////////////////////////////////////////////////
 Udp.write(vccstring,16);
 Udp.endPacket();
 
 delay(1);
 

digitalWrite(pin_out_LED,HIGH); 
delay(50);
digitalWrite(pin_out_LED,LOW); 
delay(150);

 }
