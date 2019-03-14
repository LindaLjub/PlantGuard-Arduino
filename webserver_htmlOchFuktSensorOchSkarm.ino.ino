#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// till display
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2c adress

// denna kod är till fuktsensor
int sensor_pin = A0; // Soil Sensor input at Analog PIN A0
int output_value = 0;  // for storing the output of the sensor.

// planta 2
int sensor_pin2 = A1; // Soil Sensor input at Analog PIN A1
int output_value2 = 0;  // for storing the output of the sensor.

// test vattenlampa
int vattenLampa1 = 12;
int vattenLampa2 = 13;

int RGB3 = 9; // blå
int RGB1 = 10;
int RGB2 = 11; // röd


// till wifi
SoftwareSerial SerialWifi(6, 7);
//char ssid[] = "NETGEAR";
//char pass[] = "aabbccddee";

//char ssid[] = "Lindas iPhone";
//char pass[] = "Bucko5165";

//char ssid[] = "TN_24GHz_BA50C7";
//char pass[] = "LWUDRKJYDU";

const char ssid[] = "STI Student";
const char pass[] = "STI1924stu";

int status = WL_IDLE_STATUS;
WiFiEspServer server(80);
RingBuffer buf(8);

void setup() {

  pinMode(vattenLampa1, OUTPUT);
  pinMode(vattenLampa2, OUTPUT);

  pinMode(RGB1, OUTPUT);
  pinMode(RGB2, OUTPUT); // RÖD
  pinMode(RGB3, OUTPUT); // BLÅ

  
// till display
  // Setup size of LCD 20 characters and 2 lines
  lcd.begin(20,2);
  // Back light on
  lcd.backlight();

  Serial.begin(115200);
  SerialWifi.begin(9600);
  WiFi.init(&SerialWifi);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    server.begin();
  }
    printWifiStatus();
    
}

void loop() {

          // till sensor
        output_value = analogRead(sensor_pin); // hämtar in värde från sensor
        output_value = map(output_value,1080,130,0,100); // we will map the output values to 0-100, because the moisture is measured in percentage.

       output_value2 = analogRead(sensor_pin2); // hämtar in värde från sensor
       output_value2 = map(output_value2,1080,130,0,100); // we will map the output values to 0-100, because the moisture is measured in percent

          // When we took the readings from the dry soil, 
          //then the sensor value was 550 and in the wet soil, the sensor value was 10. So, we mapped these values to get the moisture
           delay(200);

    lcd.setCursor(0,0);
    lcd.print("Plant1: ");
    lcd.print(output_value);
    lcd.print("%");

    if (output_value < 60)
    {
    lcd.print(" BAD  ");
      analogWrite(11, 255);
      analogWrite(10, 0);
      analogWrite(9, 0);
  
    digitalWrite(vattenLampa1, HIGH);
    delay(5000);
    digitalWrite(vattenLampa1, LOW);
    
    }
    else
    {
      lcd.print(" GOOD ");   
      digitalWrite(vattenLampa1, LOW);        
    }

    lcd.setCursor(0,1);
    lcd.print("Plant2: ");
    lcd.print(output_value2);
    lcd.print("%");

    if (output_value2 < 60)
    {
    lcd.print(" BAD  ");

     analogWrite(11, 255);
     analogWrite(10, 0);
     analogWrite(9, 0);
      
    digitalWrite(vattenLampa2, HIGH);
    delay(5000);
    digitalWrite(vattenLampa2, LOW);
    }
    else
    {
      lcd.print(" GOOD ");
      digitalWrite(vattenLampa2, LOW);
    }

    if(output_value2 > 60 && output_value > 60)
    {
      analogWrite(11, 0);
      analogWrite(10, 255);
      analogWrite(9, 0);
    }
   
  WiFiEspClient client = server.available();
  if (client) {
    buf.init(); 
    Serial.println("New client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        buf.push(c);

        
        if (c == '\n' && currentLineIsBlank) 
        {
          Serial.println("Sending response");

          // Send a standard HTTP response header
          client.print(F(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"));
          client.print(F("<!DOCTYPE HTML>\r\n"));
          client.print(F("<html>\r\n"));
          client.print(F("<head>\r\n"));
          
          
          client.print(F("<link rel=\"stylesheet\" href=\"https://lindaljub.github.io/Css/arduino.css\">"));
          client.print(F("<link rel=\"shortcut icon\" href=\"about:blank\">"));
          client.print(F("<title>Lindino</title>\r\n"));
          client.print(F("</head>\r\n"));
          client.print(F("<body>"));
         
          // div container börjar
          client.print(F("<div class=\"grid-container\">"));

          client.print(F("<div class=\"header\">"));
          client.print(F("<h1>Lindas Arduino</h1>"));
          client.print(F("</div>"));
         
          client.print(F("<div class=\"sida1\">"));
          client.print(F("<hr><h3>Plant ONE</h3><hr>"));
          client.print(F("<p><b>Status: </b>"));
          
              client.print(F(" Moisture:  "));
              client.print(output_value); 
              client.print(F("%"));
              client.println(F("</p>"));

              client.print(F("<p><b>Message: </b>"));
               if(output_value < 60)
                 {
                   client.print(F(" Give me water.</p>"));
                   delay(1000); 
                 }
                 
                 else
                  {
                    client.print(F(" I feel fine!</p>"));
                    delay(1000);
                  }
              
          client.print(F("</div>"));

          client.print(F("<div class=\"sida2\">"));
          client.print(F("<hr><h3>Plant TWO</h3><hr>"));
          client.print(F("<p><b>Status: </b>"));
              client.print(F(" Moisture:  "));
              client.print(output_value2); 
              client.print(F("%"));
              client.println(F("</p>"));
              
          client.print(F("<p><b>Message: </b>"));

                if(output_value2 < 60)
                 {
                   client.print(F(" Give me water.</p>"));
                   delay(1000); 
                 }
                 
                 else
                  {
                    client.print(F(" I feel fine!</p>"));
                    delay(1000);
                  }
          client.print(F("</div>"));

          client.print(F("<div class=\"planta1\"></div>"));
          client.print(F("<div class=\"planta2\"></div>")); 
            
          // här slutar div container
          client.print(F("</div></body></html>"));
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }


    delay(3000);

    client.stop();
    Serial.println(F("Client disconnected"));
  }
}


void printWifiStatus()
{
    // print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");

}
