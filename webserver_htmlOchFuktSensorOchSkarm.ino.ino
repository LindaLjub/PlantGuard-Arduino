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

// till wifi
SoftwareSerial SerialWifi(6, 7);
//char ssid[] = "NETGEAR";
//char pass[] = "aabbccddee";

//char ssid[] = "Lindas iPhone";
//char pass[] = "Bucko5165";

char ssid[] = "TN_24GHz_BA50C7";
char pass[] = "LWUDRKJYDU";

//char ssid[] = "STI Student";
//char pass[] = "STI1924stu";

int status = WL_IDLE_STATUS;
WiFiEspServer server(80);
RingBuffer buf(8);

void setup() {
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
    }
    else
    {
      lcd.print(" GOOD ");
    }

    lcd.setCursor(0,1);
    lcd.print("Plant2: ");
    lcd.print(output_value2);
    lcd.print("%");

    if (output_value2 < 60)
    {
    lcd.print(" BAD  ");
    }
    else
    {
      lcd.print(" GOOD ");
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
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n");
          client.print("<!DOCTYPE HTML>\r\n");
          client.print("<html>\r\n");
          client.print("<head>\r\n");
          
          
          client.print("<link rel=\"stylesheet\" href=\"https://lindaljub.github.io/Css/arduino.css\">");
            
          //client.print("<link rel=\"stylesheet\" href=\"http://primat.se/services/css/arduino.css\">");
          client.print("<link rel=\"shortcut icon\" href=\"about:blank\">");
          client.print("<title>AServer</title>\r\n");
          client.print("</head>\r\n");
          client.print("<body>");
         
          // div container börjar
          client.print("<div class=\"grid-container\">");

          client.print("<div class=\"header\">");
          client.print("<h1>Lindas Arduino</h1>");
          client.print("</div>");
         
          client.print("<div class=\"sida2\">");
          client.print("<hr><h3>Plant ONE</h3><hr>");
          client.print("<p><b>Status: </b>");
          
              client.print(" Moisture:  ");
              client.print(output_value); 
              client.print("%");
              client.println("</p>");

              client.print("<p><b>Message: </b>");
               if(output_value < 60)
                 {
                   client.print(" Give me water.</p>");
                   delay(1000); 
                 }
                 
                 else
                  {
                    client.print(" I feel fine!</p>");
                    delay(1000);
                  }
              
          client.print("</div>");

          client.print("<div class=\"sida1\">");
          client.print("<hr><h3>Plant TWO</h3><hr>");
          client.print("<p><b>status: </b>");
              client.print(" Moisture:  ");
              client.print(output_value2); 
              client.print("%");
              client.println("</p>");
              
          client.print("<p><b>Message: </b>");

                if(output_value2 < 60)
                 {
                   client.print(" Give me water.</p>");
                   delay(1000); 
                 }
                 
                 else
                  {
                    client.print(" I feel fine!</p>");
                    delay(1000);
                  }
          client.print("</div>");

          client.print("<div class=\"planta1\"></div>");
          client.print("<div class=\"planta2\"></div>"); 
            
          // här slutar div container
          client.print("</div>");
          client.print("</body>");
          client.print("</html>");
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
    Serial.println("Client disconnected");
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
