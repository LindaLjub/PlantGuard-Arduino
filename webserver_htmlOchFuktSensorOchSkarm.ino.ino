#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// till LED-display
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

// denna kod är till fuktsensor 1 för planta 1
int sensor_pin = A0;   // Analog PIN A0
int output_value = 0;  // sparar värdet av sensorn.

// denna kod är till fuktsensor 2 för planta 2
int sensor_pin2 = A1;   // Analog PIN A1
int output_value2 = 0;  // sparar värdet av sensorn.

// vattenlampor, lyser blått när det vattnas.
int vattenLampa1 = 12;
int vattenLampa2 = 13;

// RGB-lampa, 
// lyser grön/rött beroende på om plantorna är torra/fuktiga.
int RGB3 = 9;  // blå
int RGB1 = 10; // grön
int RGB2 = 11; // röd


int count1 = 0;
int count2 = 0;

// Styr loopen om det ska vattnas eller ej.
bool water = true;


// Till wifi
SoftwareSerial SerialWifi(6, 7);

// DITT EGET WIFI & KOD
const char ssid[] = "XXXXX";
const char pass[] = "XXXXX";

int status = WL_IDLE_STATUS;
WiFiEspServer server(80);
RingBuffer buf(8);

void setup() {

  // deklarationer av pins
  pinMode(vattenLampa1, OUTPUT);
  pinMode(vattenLampa2, OUTPUT);
  
  pinMode(RGB1, OUTPUT); // GRÖN
  pinMode(RGB2, OUTPUT); // RÖD
  pinMode(RGB3, OUTPUT); // BLÅ

  // till vattenventil
  pinMode(2, OUTPUT);


  // Till display
  lcd.begin(20,2);   // Setup size of LCD 20 characters and 2 lines.
  lcd.backlight();   // Back light on


  // WIFI kod
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
    
  // skriver ut wifi status och ip-adress
  printWifiStatus();
    
}

void loop() {

  // Sensornerna 1 & 2
  output_value = analogRead(sensor_pin); // hämtar in värde från sensor1
  output_value2 = analogRead(sensor_pin2); // hämtar in värde från sensor2

  // mappar ut värdet så att det blir mellan 0-100, för att fukten mäts i procent.
  // Man mappar ut värdet genom att ta värdet men får ut av helt torr(1080) jord och helt blöt jord(130).
  output_value2 = map(output_value2,1080,130,0,100); 
  output_value = map(output_value,1080,130,0,100); 
  
  delay(200); // lite tid att hinna ta in sensorvärden.

    // LED-display, printa ut på första raden - värdet av fuktsensor 1.
    lcd.setCursor(0,0);
    lcd.print("Plant1: ");
    lcd.print(output_value);
    lcd.print("%");

          // RGB-lampa, om det är för torrt lyser lampan rött, och det står BAD.
          // loopen räknar ned, sedan vattnas det om loopen INTE avbryts.
          // Detta för att man ska kunna ta ut sensorn ur jorden utan att den vattnas direkt.
          if (output_value < 60)
          {
            water = true;         // Ok att (kanske)vattna.
            lcd.print(" BAD  ");
            analogWrite(11, 255); // skapar rött ljus.
            analogWrite(10, 0);
            analogWrite(9, 0);
            lcd.clear();
                  // räknar ned 15 sek innan den vattnar.
                  for(int i = 15; i > 0; i--)
                  {
                    lcd.print("       ");
                    lcd.print(i);
                    lcd.print("  ");
                    delay(1000);

                     // hämtar in värdet för att se ifall vattningen ska avbrytas.
                     // detta ifall man bara vill ta ut sensorn ur jorden en snabb stund för att exempelvis flytta plantan.
                     output_value = analogRead(sensor_pin); // hämtar in värde från sensor1
                     output_value = map(output_value,1080,130,0,100); // mappar ut värdet (som beskrivet innan).
                     delay(200);
                      
                            // stoppa vattningen, om fukten åter igen är över 60.
                            if(output_value > 60)
                            {
                              water = false;    // vattning INTE ok
                              i = 0;            // Stoppar loopen
                            }
                            
                     lcd.clear(); // rensar display från nedräkning.
                  }
                
            // vattna bara om loopen inte avbröts
            if(water == true)
            {
                //den blå vattenlampan lyser och displayen visar att det vattnas.
                digitalWrite(vattenLampa1, HIGH); 
                lcd.clear();
                lcd.print("WATERING PLANT 1");
                
                // vattenventil, öppen 10 sek sedan stäng.
                   digitalWrite(2 , HIGH);
                   Serial.println("open");
                   delay(10000);
                   digitalWrite(2, LOW);
   

                count1++;   // räknar hur många gånger plantan har vattnat denna planta sen arduinon startade.
                delay(5000);
                digitalWrite(vattenLampa1, LOW);
             }        
        }
        
    // om loopen avbröts, fukt = ok. Fortsätt som innan.
    else
    {
      lcd.print(" GOOD ");   
      digitalWrite(vattenLampa1, LOW);        
    }

    
    // LED-display, printa ut på andra raden - värdet av fuktsensor 2.
    lcd.setCursor(0,1);
    lcd.print("Plant2: ");
    lcd.print(output_value2);
    lcd.print("%");

        // RGB-lampa, om det är för torrt lyser lampan rött, och det står BAD.
        // loopen räknar ned, sedan vattnas det om loopen INTE avbryts.
        // Detta för att man ska kunna ta ut sensorn ur jorden utan att den vattnas direkt.
        if (output_value2 < 60)
        {
          water = true;         // Ok att (kanske)vattna.
          lcd.print(" BAD  ");
          analogWrite(11, 255);  // skapar rött ljus
          analogWrite(10, 0);
          analogWrite(9, 0);
          lcd.clear();
              // räknar ned 15 sek innan den vattnar.
              for(int i = 15; i > 0; i--)
              {
                   lcd.print("       ");
                   lcd.print(i);
                   lcd.print("  ");
                   delay(1000);

                       // hämtar in värdet för att se ifall vattningen ska avbrytas.
                       // detta ifall man bara vill ta ut sensorn ur jorden en snabb stund för att exempelvis flytta plantan.
                       output_value2 = analogRead(sensor_pin2); // hämtar in värde från sensor2
                       output_value2 = map(output_value2,1080,130,0,100); 
                       delay(200);
                       
                               // stoppa vattningen, om fukten åter igen är över 60.
                              if(output_value2 > 60)
                              {
                                water = false;  // vattning INTE ok
                                i = 0;          // Stoppar loopen
                              }
                              
                  lcd.clear();
              }
      
          // vattna bara om loopen inte avbröts
          if(water == true)
          {
            digitalWrite(vattenLampa2, HIGH);
            lcd.print("WATERING PLANT 2");
            count2++;   // räknar hur många gånger plantan har vattnat denna planta sen arduinon startade.
            delay(5000);
            digitalWrite(vattenLampa2, LOW);
          }
      }
      
      // om loopen avbröts, fukt = ok. Fortsätt som innan.
      else
      {
        lcd.print(" GOOD ");
        digitalWrite(vattenLampa2, LOW);
      }

    // Om bägge plantor är tillräkligt fuktiga så lyser RGB lampan grönt.
    if(output_value2 > 60 && output_value > 60)
    {
      analogWrite(11, 0);
      analogWrite(10, 255);
      analogWrite(9, 0);
    }

  // WIFI kod
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

           // Webbsever, skapar en html sida.
          client.print(F("<!DOCTYPE HTML>\r\n"));
          client.print(F("<html>\r\n"));
          client.print(F("<head>\r\n"));
          
          // hämtar in CSS till html-sidan via internet.
          client.print(F("<link rel=\"stylesheet\" href=\"https://lindaljub.github.io/Css/arduino.css\">"));
          client.print(F("<link rel=\"shortcut icon\" href=\"about:blank\">"));
          client.print(F("<title>Lindino</title>\r\n"));
          client.print(F("</head>\r\n"));
          client.print(F("<body>"));
         
          // div container börjar här
          client.print(F("<div class=\"grid-container\">"));

          client.print(F("<div class=\"header\">"));
          client.print(F("<h1>Lindas Arduino</h1>"));
          client.print(F("</div>"));

          // planta 1
          client.print(F("<div class=\"sida1\">"));
          client.print(F("<hr><h3>Plant ONE</h3><hr>"));
          client.print(F("<blockquote><blockquote><b>Status: </b>"));

               // visar upp info om fukt och antal ggr vattnad.
              client.print(F(" Moisture:  "));
              client.print(output_value); 
              client.print(F("%"));
              client.print(F("<br>Watered "));
              client.println(count1);
              client.print(F(" times since Arduino was started."));
              client.println(F("</blockquote></blockquote>"));
              client.print(F("<blockquote><blockquote><b>Message: </b>"));
              
                  // visar upp ett meddelande beroende på om det är Ok med fukt eller ej.
                 if(output_value < 60)
                 {
                     client.print(F(" I dont feel so good!"));
                     client.print(F("<br>Give me water.</blockquote></blockquote>"));
                     delay(1000); 
                 }
                 else
                 {
                     client.print(F(" I feel fine!"));
                     client.print(F("<br>No water needed.</blockquote></blockquote>"));
                     delay(1000);
                 }
              
          client.print(F("</div>"));
          
          // planta 2
          client.print(F("<div class=\"sida2\">"));
          client.print(F("<hr><h3>Plant TWO</h3><hr>"));
          client.print(F("<blockquote><blockquote><b>Status: </b>"));

              // visar upp info om fukt och antal ggr vattnad.
              client.print(F(" Moisture:  "));
              client.print(output_value2); 
              client.print(F("%"));
              client.print(F("<br>Watered "));
              client.println(count2);
              client.print(F(" times since Arduino was started."));
              client.println(F("</blockquote></blockquote>"));
              client.print(F("<blockquote><blockquote><b>Message: </b>"));
          
                 // visar upp ett meddelande beroende på om det är Ok med fukt eller ej.
                if(output_value2 < 60)
                 {
                   client.print(F(" I dont feel so good!"));
                   client.print(F("<br>Give me water.</blockquote></blockquote>"));
                   delay(1000); 
                 }
                 else
                 {
                    client.print(F(" I feel fine!"));
                    client.print(F("<br>No water needed.</blockquote></blockquote>"));
                    delay(1000);
                 }
                 
          client.print(F("</div>"));
          // boxar med bilder på plantorna.
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
