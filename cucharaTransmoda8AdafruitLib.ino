
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Update these with values suitable for your network.

const char* ssid = "yourSSID";
const char* password = "YourPassword";
const char* mqtt_server = "192.168.5.108";//Here the IP address of the mqtt broker.

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
long counter = 0;
int table [80];
String msgStr;
float mapMsg;
char charMsgTotal[5];

float acceXant = 0;
float acceYant = 0;
float acceZant = 0;
bool pub = false;
float acceTotalAnt = 0;

const int minMidi = 0;
const int maxMidi = 127;
const int sampleTime = 21;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);// Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!Serial)
    delay(10);
  Serial.println("Adafruit MPU6050 test!");
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  //Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G); //puede ser 2, 4, 8, 16
  mpu.setGyroRange(MPU6050_RANGE_1000_DEG);//puede ser 250, 500, 1000,2000
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);//puede ser 5, 10 ,21, 44, 94,184,260

  for (int i = 0; i < 79; i++) {
    if (i < 20) {
      table[i] = 20;
    } else if (i >= 20 && i < 40) {
      table[i] = 70;
    } else if (i >= 40 && i <= 80) {
      table[i] = 120;
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > sampleTime) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, charMsgTotal, value);
    // Serial.print("Publish message: ");
    // Serial.println(msg);
    sample();
    if (pub) {
      client.publish("acceTotal1", msg);//topic al que publicamos y mensaje, cambiamos el numero correspondiente a cada cliente
    }
  }
}

void sample() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float acceX = a.acceleration.x - acceXant;
  float acceY = a.acceleration.y - acceYant;
  float acceZ = a.acceleration.z - acceZant;
  //float acceTotal = abs(acceX) + abs(acceY) + abs(acceZ); //sumamos los valores absolutos de diferencia de aceleracion entre todos los ejes
  float acceTotal = sqrt(sq(acceX)) + sqrt(sq(acceY)) + sqrt(sq(acceZ)); //sumamos los valores absolutos de diferencia de aceleracion entre todos los ejes
  int indexTable = round(acceTotal);
  int acceTable = table[indexTable];
  mapMsg = map(acceTotal, 0, 30 , minMidi, maxMidi);//mapeamos a valores MIDI. RAW
  msgStr = String(constrain(mapMsg, minMidi, maxMidi)); //convertimos el valor a string de one
  //Serial.println(msgStr);
  msgStr.toCharArray(charMsgTotal, 5); // convertimos el string en un chart
  if (abs(acceTotal - acceTotalAnt) > 0.15) {
    pub = true;
  } else {
    pub = false;
  }
  Serial.println(acceTotal);
  acceXant = a.acceleration.x;//reescribios el valor anterior del acc eje
  acceYant = a.acceleration.y;
  acceZant = a.acceleration.z;
  acceTotalAnt = acceTotal;
}




void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Funcion de lectura de msj mqtt
  if ((char)payload[0] == '12') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}
