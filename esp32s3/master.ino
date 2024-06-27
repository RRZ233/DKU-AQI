#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

WiFiMulti wifiMulti;

// These are basic information for InfluxDB 
#define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "m8kcS2P4sXkAJ9bvB0SUcLZurbydl1i-2ci6qypNtKSJ8_y1hPg4OOF1lwNOk3JbxipGIqyPsVUutkcvlUn-aw=="
#define INFLUXDB_ORG "34cc098180bedebe"
#define INFLUXDB_BUCKET "test"
#define sensorName "Arduino test"
#define sensorPlace "IB2073"

// Time zone info
#define TZ_INFO "UTC8"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor(sensorName);

// WIFI_connect use SSID and Password to connect wifi.
void WIFI_Connect(const char* ID,const char* PW){
  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ID, PW);
  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to "+String(ID));

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Define wifi ssid and password
  const char* WIFI_SSID = "DKU";
  const char* WIFI_PASSWORD = "Duk3blu3!";
  WIFI_Connect(WIFI_SSID,WIFI_PASSWORD);
  sensor.addTag("Place", sensorPlace);
  Serial1.begin(9600, SERIAL_8N1, 43, 44);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Get data from salve
  while(1){
    Serial1.print('1');
    delay(3000);
    if (Serial1.available())
      break;
  }
  
  String receivedData="";
  char terminatorChar = ',';
  delay(350);
  // Check the data read from slave
  receivedData = Serial1.readStringUntil(terminatorChar);
  Serial.println(receivedData);

  // Slice the string and transform into float
  int index1 = receivedData.indexOf(' ');
  int index2 = receivedData.indexOf(' ', index1 + 1);
  int index3 = receivedData.indexOf(' ', index2 + 1);
  int index4 = receivedData.indexOf(' ', index3 + 1);

  float temperature = receivedData.substring(0, index1).toFloat();
  float humidity = receivedData.substring(index1 + 1, index2).toFloat();
  float airPressure = receivedData.substring(index2 + 1, index3).toFloat();
  float iaqIndex = receivedData.substring(index3 + 1, index4).toFloat();

  // Clear data in point
  sensor.clearFields();
  
  // Store measured value into point
  if(((temperature*humidity*airPressure*iaqIndex)!=0)&&(humidity>0)){
    sensor.addField("Temperature_c", temperature);
    sensor.addField("Humidity_percent", humidity);
    sensor.addField("Air_pressure_kPa", airPressure);
    sensor.addField("IAQ_Index", iaqIndex);
    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
  
  
  // Check WiFi connection and reconnect if needed
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("Wifi connection lost");
    }

  // Write point
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
      esp_restart();
      Serial.println("Restart device");
    }
  }
}
