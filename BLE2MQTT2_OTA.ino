#include <sstream>
#include <string>
#include <cstring>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include <EspMQTTClient.h>



int scanTime = 5; //In seconds
BLEScan* pBLEScan;
// for more detailed instructions on these settings, see the EspMQTTClient github repo
EspMQTTClient client(
  "ShinSekai",
  "<password for AP above>",
  "192.168.10.10",
  "stef",
  "my_great_password_for_mqtt",
  "myclient",
  1883
);

void onConnectionEstablished()
{
  // do nothing on connection.. I believe this is deprecated anyway but.. yeah
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    if (advertisedDevice.haveManufacturerData() && advertisedDevice.haveServiceUUID()) {
       // The serviceUUID that follows is the Inkbird Company one.. 
       // this means that we can 'auto-populate' new inkbird devices
       // as and when they come online, into mqtt. Still need to setup HA obviously
       if (strcmp(advertisedDevice.getServiceUUID().toString().c_str(), "0000fff0-0000-1000-8000-00805f9b34fb") != 0) {
          Serial.println("Serials do not match!!");
          Serial.println(advertisedDevice.getServiceUUID().toString().c_str());
       }
       
       char *pHex = BLEUtils::buildHexData(nullptr, (uint8_t*)advertisedDevice.getManufacturerData().data(), advertisedDevice.getManufacturerData().length());

       char _hum[5];
       strncpy(_hum, pHex+6, 2);
       strncpy(_hum+2, pHex+4, 2);
       _hum[4]='\0';
       float humidity = strtol(_hum, NULL, 16) / 100.0;
       //Serial.println(humidity);

       char _tmp[5];
       strncpy(_tmp, pHex+2, 2);
       strncpy(_tmp+2, pHex, 2);
       _tmp[4]='\0';
       int tmp_temperature = strtol(_tmp, NULL, 16);
       int temperature_bits = 16;
       if (tmp_temperature & (1 << (temperature_bits-1))) {
         tmp_temperature -= 1 << temperature_bits;
       }
       float temperature = tmp_temperature / 100.0;
       //Serial.println(temperature);

       char _bat[3];
       strncpy(_bat, pHex+14, 2);
       _bat[2]='\0';
       int battery = strtol(_bat, NULL, 16);
       //Serial.println(battery);


       char temperature_out[255];
       sprintf(temperature_out, "bluetooth/inkbird/%s/temperature", advertisedDevice.getAddress().toString().c_str());
       char temperature_value[7];
       sprintf(temperature_value, "%.2f", temperature);
       client.publish(&temperature_out[0], &temperature_value[0]);

       char humidity_out[255];
       sprintf(humidity_out, "bluetooth/inkbird/%s/humidity", advertisedDevice.getAddress().toString().c_str());
       char humidity_value[5];
       sprintf(humidity_value, "%.2f", humidity);
       client.publish(&humidity_out[0], &humidity_value[0]);

       char battery_out[255];
       sprintf(battery_out, "bluetooth/inkbird/%s/battery", advertisedDevice.getAddress().toString().c_str());
       char battery_value[4];
       sprintf(battery_value, "%d", battery);
       client.publish(&battery_out[0], &battery_value[0]);
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater("/"); // Incase we ever need to update OTA .. if we don't .. jst remark this out
  BLEDevice::init("myBLEScan");
}

void bleScan() {
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(3000);   // 
  pBLEScan->setWindow(2900);     // less or equal setInterval value
    
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->stop();
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}


int cnt=0;

void loop() {
  client.loop();
  if (cnt == 0) { bleScan(); }

  cnt=cnt+1;
  
  if (cnt >= 5) { cnt=0; }

  client.loop();
  delay(5000); // or 5s
}
