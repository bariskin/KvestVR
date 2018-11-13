#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

// таймер для установки временных пауз 

unsigned long currentTime1;
unsigned long loopTime1; 
byte loop_flag1;
//*******************************************

unsigned long currentTime2;
unsigned long loopTime2; 
byte loop_flag2;


//*******************************************
// флаги включения устройств 
int flag_output_off_on_1 = 0;
int flag_output_off_on_2 = 0;
//*******************************************


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


IPAddress ip(10,0,0,13);                            //статический для вибро 
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
byte input_1_flag = 1; // флаг разрешения сигналов управления 
byte input_2_flag = 1; // флаг разрешения сигналов управления 


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


  digitalWrite(pin_out_1, LOW);
  digitalWrite(pin_out_2, LOW);
  
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
   digitalWrite(pin_out_LED,LOW);
   // ------------------------------------ LISTEN INBOUND ----------------------------------- //
   Serial.printf("Board status: Received %d bytes from %s, port: %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
   int len = Udp.read(incomingPacket, 255);
   if (len > 0) 
   {
    incomingPacket[len] = 0;
   }

   if (incomingPacket[0] == '#' && incomingPacket[1] == 'P' && incomingPacket[2] == 'I' && incomingPacket[3] == 'N') 
   { 
       // --------------------------------------- DIGITAL OUTPUT pin_out_1 ----------------------------- //
    
    if (incomingPacket[5] == '1') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { input_1_flag = 0; Serial.print("Board status: PIN_1 = DISABLED\r\n"); }
     
     if ((input_1_flag == 1) && (incomingPacket[7] == '0')) { 
        digitalWrite(pin_out_1,LOW); 
        flag_output_off_on_1 = 0; // флаг выключения первого пина
        Serial.print("Board status: PIN_1 = OFF"); 
      }
     if ((input_1_flag == 1) && (incomingPacket[7] == '1')) { 
        digitalWrite(pin_out_1,HIGH); 
        
        flag_output_off_on_1 = 1; // флаг включения первого пина
        
        Serial.print("Board status: PIN_1 = ON");
      }
    }

     // --------------------------------------- DIGITAL OUTPUT pin_out_2 ----------------------------- //
    if (incomingPacket[5] == '2') 
    { 
     if ((incomingPacket[7] == 'O') && (incomingPacket[8] == 'F') && (incomingPacket[9] == 'F')) { 
      input_2_flag = 0; Serial.print("Board status: PIN_2 = DISABLED\r\n"); 
      }
      
     if ((input_2_flag == 1) && (incomingPacket[7] == '0')) { 
        digitalWrite(pin_out_2,LOW);
         flag_output_off_on_2 = 1; // 
        Serial.print("Board status: PIN_2 = OFF"); 
      }
      
     if ((input_2_flag == 1) && (incomingPacket[7] == '1')) { 
        digitalWrite(pin_out_2,HIGH); 
        flag_output_off_on_2 = 1; // 
        Serial.print("Board status: PIN_2 = ON"); 
     }
    }
    
 
   }
 
 // ----------------------------------- ANSWER TO SERVER ----------------------------------- //
   
   Serial.printf("UDP packet contents: %s\n", incomingPacket);
   Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
   Udp.write(replyPacket);Udp.write(incomingPacket);Udp.endPacket();
 }
 else 
 {
   digitalWrite(pin_out_LED,HIGH);
 } 

//-------------------таймер выключения pin_out_1-------------------------------------
     
       if(flag_output_off_on_1 == 1){
               currentTime1 = millis();
       
            if (loop_flag1==0){
                    loopTime1 = currentTime1;  
                    loop_flag1 = 1;
                   }
            if(currentTime1 >= (loopTime1 + 10000)){      // время работы пина      pin_out_1       
                    loop_flag1 = 0;
                    flag_output_off_on_1= 0;
                    digitalWrite(pin_out_1,LOW); 
                   }   
       }

 //-------------------таймер выключения pin_out_2-------------------------------------
 
       if(flag_output_off_on_2 == 1){
            currentTime2 = millis();  
       
            if (loop_flag2==0){
                    loopTime2 = currentTime2;  
                    loop_flag2 = 1;
                   }
            if(currentTime2 >= (loopTime2 + 10000)){      // время работы пина      pin_out_1             
                    loop_flag2 = 0;
                    flag_output_off_on_2= 0;
                    digitalWrite(pin_out_2,LOW); 
                   }   
       }      
// // ----------------------------------- ANALOG VOLTAGE PACKET SEND ------------------------- //
// Serial.println();
// Serial.println();
// Serial.print("Connect to ");
// Serial.println(ssid);
// WiFi.begin(ssid, password);             // Connecting to WiFi
// --------------------------------------- DIGITAL OUTPUT ----------------------------- //

 if (( flag_output_off_on_1 == 1) && (TRIGGER1_flag == 0) )
 {
  Serial.printf("#TRIGGER1_ON");
  
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
 }

 if ((flag_output_off_on_1 == 0) && (TRIGGER1_flag == 1))
 {
 Serial.printf("#TRIGGER1_OFF");
  
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
 }

 if ((flag_output_off_on_2 == 1) && (TRIGGER2_flag == 0))
 {
  Serial.printf("#TRIGGER2_ON");
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER2_ON"); 
  Udp.endPacket();
  
  Udp.beginPacket(ip_sh2, port); 
  Udp.write("#TRIGGER2_ON"); 
  Udp.endPacket();

  Udp.beginPacket(ip_sh3, port); 
  Udp.write("#TRIGGER2_ON"); 
  Udp.endPacket();
  TRIGGER2_flag = 1;
 }

 if ((flag_output_off_on_2  == 0) && (TRIGGER2_flag == 1))
 {
   Serial.printf("#TRIGGER2_OFF");
   
  Udp.beginPacket(ip_sh1, port); 
  Udp.write("#TRIGGER2_OFF"); 
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
