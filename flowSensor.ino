#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#########################CHANGE ME#####################################

#define wifi_ssid "YOUR_SSID"                   // WLAN SSID
#define wifi_password "YOUR_WIFI_PASSWORD"      // WLAN Password

#define mqtt_server "YOUR_MQTT_SERVER_IP"       // MQTT IP Address e.g. 192.168.2.187
#define mqtt_port "1883"                        // MQTT Port
#define mqtt_user "MQTT_USER"                   // MQTT User
#define mqtt_password "MQTT_PASSWORD"           // MQTT Password


float factor = 5.0;                             //Flow Sensor Factor

#####################################################################

#define topicCurrent "flowMeter/Current_L_min"  // MQTT Topic for Current Flow
#define topicTotal "flowMeter/Total_L"          // MQTT Topic for Total Flow since last Reboot


WiFiClient espClient;
PubSubClient client(espClient);


byte interrupt = 0;
byte sensorPin = 4;
byte pulseCount;
float flowRate;
float t_l;
unsigned int f_ml;
unsigned long t_ml;
unsigned long oldTime;


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(interrupt, pulseCounter, FALLING);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


long lastMsg = 0;



void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  if ((millis() - oldTime) > 1000)
  {
    int semic;
    detachInterrupt(interrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / factor;
    oldTime = millis();
    f_ml = (flowRate / 60) * 1000;
    t_ml += f_ml;
    t_l = float(t_ml)/1000;
    Serial.print("Current flow: ");
    Serial.print(int(flowRate));
    Serial.print(".");
    semic = (flowRate - int(flowRate)) * 10;
    Serial.print(semic, DEC) ;
    Serial.print("L/min");
    Serial.print("  Total: ");
    Serial.print(t_ml);
    Serial.print("mL / ");
    Serial.print(t_l);
    Serial.println("l");
    pulseCount = 0;
    attachInterrupt(interrupt, pulseCounter, FALLING);
  }

  long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    client.publish(topicCurrent, String(flowRate).c_str(), true);
    client.publish(topicTotal, String(t_l).c_str(), true);
  }


}
void pulseCounter()
{
  pulseCount++;
}
