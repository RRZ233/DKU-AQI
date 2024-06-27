#include <bme68xLibrary.h>
#include <Wire.h>

Bme68x bme;
#define WIRE Wire


// The i2c_scanner uses the return value of
// the Write.endTransmisstion to see if
// a device did acknowledge to the address.
int i2cScanner() {
  byte error, address;
  int nDevices;
  int i2c;
  Serial.println("Scanning...");

  // Scan each device from 1 to 126
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    WIRE.beginTransmission(address);
    error = WIRE.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.println("  !");
      i2c = address;
      nDevices++;
      break;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done");
  // wait 5 seconds for next scan
  delay(5000);           
  return i2c;
}

void setup() {
  // put your setup code here, to run once:
  // Initialize I2C with specified pins
  Wire.begin();
 // Wire.begin(4, 5); // SDA, SCL
  Serial.begin(9600);
  delay(500);

  // Check I2C connection
  int i2caddress = -1;
  i2caddress = i2cScanner();
  if (i2caddress == -1) {
    Serial.println("No I2C devices found. Check connections.");
    setup(); 
  }

  // Initialize the BME680 sensor
  bme.begin(i2caddress, Wire);
  bme.setTPH();

  // Check if the sensor initialization was successful
  if (bme.status != BME68X_OK) {
    Serial.print("Could not find a valid BME680 sensor, check wiring! Error: ");
    Serial.println(bme.statusString());
    esp_restart();
  }
  //300*C for 100 ms, this is a preheat, which makes sure the right iaq measurement at the beginning.
  bme.setHeaterProf(200,100);
  bme.setOpMode(BME68X_FORCED_MODE);

  // Initialize UART1 (TX on GPIO 43, RX on GPIO 44)
  Serial1.begin(9600, SERIAL_8N1, 43, 44);
  Serial.begin(9600);

}

// This will return IAQ index basing on
// gas resistance and humidity.
float fCalulate_IAQ_Index( int gasResistance, float Humidity){
  float hum_baseline = 40.0f;
  float hum_weighting = 0.25f;
  float gas_offset = 0.0f;
  float hum_offset = 0.0f;
  float hum_score = 0.0f;
  float gas_score = 0.0f;
  float oGasResistanceBaseLine = 149598.0f;
  gas_offset = oGasResistanceBaseLine - float( gasResistance );
  hum_offset = float( Humidity ) - hum_baseline;
  // calculate hum_score as distance from hum_baseline
  if ( hum_offset > 0.0f )
  {
    hum_score = 100.0f - hum_baseline - hum_offset;
    hum_score /= ( 100.0f - hum_baseline );
    hum_score *= ( hum_weighting * 100.0f );
  } else {
    hum_score = hum_baseline + hum_offset;
    hum_score /= hum_baseline;
    hum_score *= ( 100.0f - (hum_weighting * 100.0f) );
  }
  //calculate gas score as distance from baseline
  if ( gas_offset > 0.0f )
  {
    gas_score = float( gasResistance ) / oGasResistanceBaseLine;
    gas_score *= ( 100.0f - (hum_weighting * 100.0f ) );
  } else {
    gas_score = 100.0f - ( hum_weighting * 100.0f );
  }
  return ( hum_score + gas_score );
}

void loop() {
  // put your main code here, to run repeatedly:

  // Wait until it recevices master's call
  while (Serial1.read()!='1'){
    delay(1000);
  }

  int count = 0;
  int num = 5;
  int lose = 0;
  float gasResistance =0;
  float temperature = 0;
  float humidity = 0;
  float airPressure = 0;

  // This loop will count each variables num(5 here) times
  // and caluculate average
  for (count; count < num; count++){
    bme68xData data;
    bme.setHeaterProf(200,100);
    bme.setOpMode(BME68X_FORCED_MODE);
    delayMicroseconds(bme.getMeasDur());
    bme.getAllData();

    // This will check bme situation
    if((bme.readReg(0x73)==12) && bme.fetchData()){
      bme68xData data = bme.getAllData()[0]; // Get the first data field
      gasResistance += data.gas_resistance;
      temperature += data.temperature;
      humidity += data.humidity;
      airPressure += data.pressure / 1000.0; // Convert to kPa
    }else if(bme.readReg(0x73)!=12){ // Bme fail to perform
      Serial.println(bme.readReg(0x73));
      // Record lose number
      lose += 1;
      delay(1);
    }
    delay(1000); 
  }

  gasResistance /= (num - lose);
  temperature /= (num - lose);
  humidity /= (num - lose);
  airPressure /= (num - lose);
  float iaqIndex = fCalulate_IAQ_Index(gasResistance, humidity);
  String AQIData = String(temperature)+ " "+ String(humidity)+ " "+String(airPressure)+" "+ String(iaqIndex)+",";

  // Write to master
  Serial1.print(AQIData);
  Serial.println(String(temperature)+ " "+ String(humidity)+ " "+String(airPressure)+" "+ String(iaqIndex));
  
  // Clear Serila1 buffer
  Serial1.flush();
}
