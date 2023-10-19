#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>




typedef struct {
  bool on;
  bool free;
  double value;
  long index = -1L;
} Device;
Device dev;
WiFiClientSecure wifiClient;
HTTPClient http;
DynamicJsonDocument deviceJson(2048);
bool connected = false;


const char* ssid =  ""; //Enter SSID
const char* password = ""; //Enter Password

String host = "devicescontrolremote-fqx5bw2stq-lz.a.run.app";
String hosttest = "www.testingmcafeesites.com";


String  cloudLinkConnect = "https://" + host + "/connect/light/home";
String  cloudLinkState = "https://devicescontrolremote-fqx5bw2stq-lz.a.run.app/device/state/";
String  response;
int httpCode;
ESP8266WebServer server(80);


void handle_NotFound()
{
  server.send(404, "text/html", String("<html><h1>Path not found/h1></html>") + String(ssid));
}

String SendHTML()
{
  String page = "<html>";
  page += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";

  page += "</head>\n";
  page += "<title>LED Control config data</title>\n";
  page += "<h2>Input your local WI-fi network data for configuration</h2>";
  page += "<form action=\"/config\">";
  page += "<label>ssid</label><br>\n";
  page += "<input name='ssid'><br>\n";
  page += "<label>password</label><br>\n";
  page += "<input name='pass'><br>\n";
  page += "<input type=\"submit\" value=\"submit\"/>";
  page += "</form>";
  page += "</html>";

  return page;

}

void handle_OnConnect()
{
  digitalWrite(4, 0);
  connected = false;
  server.send(200, "text/html", SendHTML());

}
void sendFirstRequest()
{
  Serial.println("State of Wifi");
  Serial.println(WiFi.status());
  wifiClient.setInsecure();

  if (!wifiClient.connect(host, 443))
  {
    Serial.print("conection has failed to establish");
    connected = false;
    sendFirstRequest();
  } else
  {
    Serial.print("CONNECTION");

    String req = String("GET ") + cloudLinkConnect + String(" HTTP/1.1\r\n") + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";

    wifiClient.print(req);
    delay(1000);
    String line = "";
    while (wifiClient.available())
    {
      line = line + wifiClient.readString();
      Serial.println(line);
      connected = true;

    }

    if (connected) {
      int nr = line.indexOf("\r\n\r\n");
      line = line.substring(nr, line.length());
      Serial.print("the found body:");
      Serial.println(line);
      dev.index = line.toInt();
      Serial.print("index of device");
      Serial.println(dev.index);

      delay(10000);


    }

  }
  if (!connected)
  {
    server.send(404, "text/html", String("<html><h1 style=\"color:red\">Configuration failed due to not being able to get id, i will try again</h1></html>") + String(ssid));
    sendFirstRequest();
  }
  else
    server.send(200, "text/html", String("<html><h1>Configuration succesful on ssid</h1></html>" ) + String(ssid));


}



void handle_config()
{
  bool correct = true;
  if ( server.hasArg( "ssid" ) && server.arg( "ssid" ) != NULL ) {
    ssid = server.arg( "ssid" ).c_str();

  }
  else
    correct = false;

  if ( server.hasArg( "pass" ) && server.arg( "pass" ) != NULL ) {
    password = server.arg( "pass" ).c_str();
  }
  else
    correct = false;

  if (correct)
  {
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");



    }
    // put your setup code here, to run once:
    Serial.print("local ip address is");
    Serial.println(WiFi.localIP());
    server.send(200, "text/html", String("<html><h1 style=\"color:green\">Configuration established, please discoonect to continue</h1></html>") + String(ssid));

    sendFirstRequest();

  }
  else
    server.send(404, "text/html", String("<html><h1 style=\"color:red\">Configuration failed due to input errors</h1></html>") + String(ssid));


}



void setup() {


  Serial.begin(9600);
  delay(1000);
  Serial.println(HIGH);
  Serial.println(LOW);
  Serial.println(OUTPUT);
  pinMode(4, OUTPUT_OPEN_DRAIN);
  pinMode(16,OUTPUT);
  delay(1000);
  digitalWrite(16,LOW);
  delay(5000);
  digitalWrite(16,HIGH);
  
  Serial.print("connecing to");
  Serial.println(ssid);
  IPAddress ip_server(192, 0, 0, 0);
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP("SmartLEDAlex");
  // WiFi.softAPConfig(ip_server,ip_server,IPAddress(255,255,255,0));
  delay(500);
  Serial.print("stupid");


  IPAddress ip = WiFi.softAPIP();
  Serial.print("IP adress of acces_point is:");
  Serial.println(ip);

  server.on("/", handle_OnConnect);
  server.on("/config", handle_config);
  server.onNotFound(handle_NotFound);

  server.begin();




}
int tries = 10;
int current = 0;

void getData() {
  if (connected)
  {



    if (!wifiClient.connect(host, 443))
    {
      Serial.print("conection has failed to establish");
      if (current >= tries)

      { Serial.print("can't connect need to configure again");
        connected = false;
        current = 0;
      } else
      {
        current += 1;
        delay(200);
      }


    } else
    {
      Serial.print("FIND STATE");

      String req = String("GET ") + cloudLinkState + String(dev.index) + String(" HTTP/1.1\r\n") + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";

      wifiClient.print(req);
      delay(200);
      String line = "";
      while (wifiClient.available())
      {
        line = line + wifiClient.readString();
        //Serial.println(line);

      }
      //      Serial.println(line);
      int nr = line.indexOf("\r\n\r\n");



      line = line.substring(nr, line.length());
      nr = line.indexOf("{");
      int nr2 = line.lastIndexOf("}") + 1;
      line = line.substring(nr, nr2);
      Serial.print("body found:");
      Serial.println(line);
      Serial.print("end body");

      deviceJson.clear();


      deserializeJson(deviceJson, line);



      dev.on = deviceJson["on"];
      dev.free = deviceJson["free"];
      dev.value = deviceJson["value"];
      Serial.println("device state:");
      if (dev.free)
        Serial.print("free");
      else
        Serial.print("used");

      Serial.print(",");

      if (dev.on)
        Serial.print("on");
      else
        Serial.print("off");

      Serial.print(",");

      Serial.println(dev.value);
      String output;
      serializeJson(deviceJson, output);
      Serial.print("Received message");
      Serial.println(output);



      //DESERIALIZARE  SI SETARE
      if (dev.free || (!dev.free && dev.on))
      {


        Serial.print("HIGH");
        //pinMode(4, OUTPUT);
        digitalWrite(4, LOW);


      }

      else
      {
        Serial.print("LOW");
        digitalWrite(4, HIGH);



      }




    }


  }

}







void loop() {
  delay(500);
  server.handleClient();
  getData();
}
