#include <Arduino.h>
#include <SPI.h>
#include "EthernetENC.h"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 16, 2), remote_ip(192,168,16,1);

unsigned int localPort = 8001;      // local port to listen on
char packetBuffer[500];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\n";        // a string to send back
//EthernetUDP Udp;  // An EthernetUDP instance to let us send and receive packets over UDP
EthernetServerPrint Serv = EthernetServerPrint(2000);
EthernetClient client; // объект клиент
boolean clientAlreadyConnected = false; // признак клиент уже подключен

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // pin 13 LED - PB5/SCK used in SPI! so no control
  Serial.begin(9600);
  Ethernet.init(10);  // CS pin
  Ethernet.begin(mac, ip);

  delay(1000);  
  Serv.begin();
  ////Udp.begin(localPort);
  //Serial.print("Listen on port: ");
  //Serial.println(localPort, DEC);
}

void loop() {
  static unsigned char LED_st = 0;
  static unsigned long LED_time = 0, LED_time_old = 0;
  static unsigned long UDP_time = 0, UDP_time_old = 0;

  LED_time = millis();
  if (LED_time > LED_time_old + 500)
  {
    LED_time_old = LED_time;
    if (LED_st == 0)
    {
      LED_st = 1;
      //digitalWrite(LED_BUILTIN, HIGH);
      //Serial.println("LED is on");
      //Udp.beginPacket(remote_ip, 8001);
      //Udp.write("LED is on\n");
      //Udp.endPacket();
    } else
    {
      LED_st = 0;
      //digitalWrite(LED_BUILTIN, LOW);
      //Serial.println("LED if off");
      //Udp.beginPacket(remote_ip, 8001);
      //Udp.write("LED is off\n");
      //Udp.endPacket();
    }
  }
    
  UDP_time = millis();
  if (UDP_time > UDP_time_old + 100)
  {
    UDP_time_old = UDP_time;

    // if there's data available, read a packet
  /*  int packetSize = Udp.parsePacket();
    if (packetSize) {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      IPAddress remote = Udp.remoteIP();
      for (int i=0; i < 4; i++) {
        Serial.print(remote[i], DEC);
        if (i < 3) {
          Serial.print(".");
        }
      }   
      Serial.print(", port ");
      Serial.println(Udp.remotePort());

      // read the packet into packetBufffer
      //Udp.read(packetBuffer, 500);
      Serial.println("Contents:");
      Serial.println(packetBuffer);

      // send a reply to the IP address and port that sent us the packet we received
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(ReplyBuffer);
      Udp.endPacket();
    }*/

    client = Serv.available(); // ожидаем объект клиент
    if (client)
    {
      // есть данные от клиента
      if (clientAlreadyConnected == false) {
        // сообщение о подключении
        Serial.println("Client connected");
        client.println("Server ready"); // ответ клиенту
        clientAlreadyConnected = true;
      }
    }

    if (client.available() > 0)
    {
      char chr = client.read(); // чтение символа
      Serv.write(chr); // передача клиенту
      Serial.write(chr);
    }
  }
}