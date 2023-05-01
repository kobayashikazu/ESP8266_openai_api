#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "Your network SSID";
const char* password = "Your network password";
const char* apiKey = "openAI API Key";

ESP8266WebServer server(80);

void handleRoot() {
  //The prompt for this language model was "Where is the capital of Japan?"
  String inputText = "Where is the capital of Japan?";
  String url = "https://api.openai.com/v1/completions";
  //Use the language model "text-davinci-003" this time
  String payload = "{\"prompt\":\"" + inputText + "\",\"max_tokens\":100, \"model\": \"text-davinci-003\"}";

  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.openai.com", 443)) {
    Serial.println("connection failed");
    return;
  }

  client.println("POST " + String(url) + " HTTP/1.1");
  client.println("Host: api.openai.com");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(payload.length()));
  client.println("Authorization: Bearer " + String(apiKey));
  client.println("Connection: close");
  client.println();
  client.println(payload);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }

  String response = "";
  while (client.available()) {
    char c = client.read();
    response += c;
  }

  client.stop();

  DynamicJsonDocument jsonDoc(1024);
  deserializeJson(jsonDoc, response);
  String outputText = jsonDoc["choices"][0]["text"];

  String page = "<html><body><h1>" + outputText + "</h1></body></html>";
  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    Serial.flush();
  }

  Serial.println("Connected to WiFi");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.flush();

  server.on("/", handleRoot);

  server.begin();
}

void loop() {
  server.handleClient();
}
