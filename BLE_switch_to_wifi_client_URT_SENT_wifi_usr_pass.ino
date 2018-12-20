/*
     BLE SWITCH TO WIFI CLIENT MODE BLE SNT WIFI USER NAME AND PASSWORD
     
     Waitng For Bluetooth client ( for 5 secound). Bluetooth client Application Setup wifi Credentials in EEPROM 
     Bletooth can communicate with this device.
     When Bluetooth client is not connected for 10 secound , Device will auto connected to the wifi network (ssid ,password already saved in eeprom) 
     Deviced will communicate with ip address when it wifi range  
    
    1. DEEP SLEEP AFTER SUCCESFULL DATA SENDING THROUGH BLE
    2. GPIO 33 PULL HIGH to WAKEUP AND COMMUNICXATE OVER BLE
    3. IF BLE CLIENT DNT CONNECT FOR 10 Sec AUTO CONNECT TO WIFI WHICH ID , PASS SEND BY BLE CLIENT  
    4. EEPROM WIFI & PASS SAVE 
    5. WHEN BLE IS CONNECTED WIFI CLIENT WILL OFF AND VISEVARSA
    6. Read ADC Battery Voltage & Tranismit it Through BLE And Webserver  
    7. BUZZER PLAY for 1 sec when it tigger
    KEYWORD : EEPROM , ESP32 , DEEP_SLEEP , TOUCH_WAKEUP , RTC_WAKEUP ,Bluetooth_LOW_ENERGY, ESP32_SERVER , ADC (Battery voltage);
    code update : 27 OCT 18
    
*/


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>

#define BUZZER 5  //BUZZER PIN


//Battery ADC
#define ADC 34  //BUZZER PIN

double ADCValue; 
String webString="";     // String to display 




// BLUETOOTH UART UUID

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// BlE UART END






uint64_t chipid;  //UNIQE CHIP ID
char ble[100];  //BLE chip id varible

// DEEP SLEEP

#define BUTTON_PIN_BITMASK 0x200000000  // DEEP SLEEP WAKE PIN GPIO 33
#define TIME_TO_SLEEP 10
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

//DEEP SLEEP 



unsigned long notConnectedSince = 0;
bool isWiFiCodeRunning = false;



WebServer server(80);     //WiFi Server will start Port on 80

//wifi webserver

void handleRoot() {
   
   webString= " <h1> Welcome To Smart Glass Web Portal </h1> <p>Find My Glass &nbsp;&nbsp;<a href=/on>CLICK ME</a> </p> <P>Battery ADC : &nbsp;" +String(ADCValue)+"%";
   server.send(200, "text/html",webString);
 
 //  server.send(200, "text/html", "Find SMART GLASS <a href=/on>CLICK ME</a>");
 
}



//WEBSERVER BUZZER STATE CHECK
bool ledState = LOW;
int j=0;

void handleClick(){
  for (j=0 ; j<1000; j++){         //BUZZER PIN 5
  digitalWrite (BUZZER, HIGH);
  delayMicroseconds(200);
  digitalWrite (BUZZER, LOW);
  delayMicroseconds(200);
  }

  ledState = !ledState;
  digitalWrite(BUZZER, ledState);
  server.send(200, "text/html", "<script>window.location ='/'</script>");
}



//WIFI SETUP 




void setupWiFi(void) {

  pinMode(BUZZER, OUTPUT);
  
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid,pass);
  connectNetDetails();
  Serial.println("");
// Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  server.on("/", handleRoot);
  server.on("/on", handleClick);
  server.begin();
}

//wifi loop


void loopWiFi(void) {
  
  if (WiFi.status() == WL_CONNECTED ) {
  server.handleClient();
  Serial.print("Wi-Fi Connected IP:");
  Serial.println(WiFi.localIP());
  ADCValue = analogRead(ADC);
  Serial.printf("Battery voltage(ADC): %lf \n  ", ADCValue);
} 

}



//End of WiFi Section

//BLE SECTION


unsigned long firstCheck = 0;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t txValue = 0;


// CHECK THE BLE CLIENT

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      WiFi.mode(WIFI_OFF);
      isWiFiCodeRunning = false;
      notConnectedSince = 0;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

int i=0;

//BLE RECCEIVED FROM APPLICATION

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite (BLECharacteristic * pCharacteristic) {
      std :: string rxValue = pCharacteristic-> getValue ();
      Serial.println (rxValue [0]);

      if (rxValue.length ()> 0) {
        parseCmd(rxValue); //function for wifi id & password
        Serial.println ("*********");
        Serial.print ("TIGGER THE BUZZER (PRESS A ON & Press B OFF):");

        for (int i = 0; i <rxValue.length (); i ++) {
          Serial.print (rxValue [i]);
        }
        Serial.println ();
        Serial.println ("*********");
      }

          // Process the character received from the application. If A turns on the LED. B turns off the LED
      if (rxValue.find ("A") != -1) { 
        Serial.println ("Turning ON BUZZER!");
        for (i=0 ; i<1000; i++){         //BUZZER PIN 5
        digitalWrite (BUZZER, HIGH);
        delayMicroseconds(200);
        digitalWrite (BUZZER, LOW);
        delayMicroseconds(200);
        }
      }
      else if (rxValue.find ("B") != -1) {
        Serial.println ("Turning OFF BUZZER!");
        digitalWrite (BUZZER, LOW);
      }  

    
    }
};




// function for ble app sent wifi id & password


void parseCmd(std::string rxValue)
{
  if(rxValue[0] == 'A')
  {
    // turn on buzzer
  }
  else if(rxValue[0] == 'B')
  {
    // turn off buzzer
  }
  else if(rxValue[0] == 'C')
  {
    // WiFi netid & password, format    CNETID:NETPASS
    char ssid[20]={};
    char pass[20]={};

    bool isNetID = true;
    int j = 0;
    int k = 0;
    
    for (int i = 1; i < rxValue.length(); i++)
    {
      if(isNetID)
      {
        if(rxValue[i] != ':')
        {
          ssid[j++] = rxValue[i];
        
        }
        else
        {
          ssid[j++] = NULL;
          isNetID = false;
        }
      }
      else
      {
        pass[k++] = rxValue[i];
       
      }

    }
    Serial.println("Received SSID & PASS From App:");
    
    Serial.println(ssid);
    Serial.println(pass);
    saveNetDetails(ssid,pass);
    WiFi.begin(ssid,pass);
    setupWiFi();
  }
}



//EEPROM DATA SAVE
 


int WIFI_SSID_ADDRS = 0;
int WIFI_PASS_ADDRS = 100;

void saveNetDetails(String ssid, String pass)
{
  EEPROM.begin(512);
  EEPROM.writeString(WIFI_SSID_ADDRS, ssid);
  EEPROM.writeString(WIFI_PASS_ADDRS, pass);
  EEPROM.commit();
  EEPROM.end();
}


//EEPROM DATA SAVE AND VALUE PASS

void connectNetDetails()
{
  EEPROM.begin(512);
  String ssid = EEPROM.readString(WIFI_SSID_ADDRS);
  String pass = EEPROM.readString(WIFI_PASS_ADDRS);
  EEPROM.end();

  char cssid[22] = {0};
  char cpass[22] = {0};
  
  ssid.toCharArray(cssid, ssid.length()+1);
  pass.toCharArray(cpass, pass.length()+1);
  
  Serial.println("Stored ssid & Pass");
  Serial.println(cssid);
  Serial.println(cpass);

  WiFi.begin(cssid, cpass);
}










// DEEP SLEEP


//BLE CONFIG

void setup() {
  Serial.begin(115200);
  
  //deep sleep
  
  pinMode(GPIO_NUM_33, INPUT_PULLDOWN);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //  esp_deep_sleep_start();
  
  //deep sleep
  
  {
  pinMode (BUZZER, OUTPUT); // BUZZER PIN
  //chipid=ESP.getEfuseMac();
 // sprintf(ble, "SmartGlass%llu", chipid); 
  // Create the BLE Device
 // BLEDevice::init(ble);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

}



// BLE DATA SENDING LOOP




void loop() {

  if (deviceConnected) {
    //BLE Connected with phone
    notConnectedSince = 0;

    
    // Read ADC Battery Voltage AnalogPin 34
   
    int ADCValue = analogRead(ADC);
   
    Serial.printf("BLE CONNECTED (Goto Deep sleep after 10 sec) & RTC WAKEUP 5 sec ADC:: %d \n  ", ADCValue);

    char buffer[5];
    itoa (ADCValue,buffer,10);
      
    pCharacteristic->setValue(buffer);
    pCharacteristic->notify();
   
    

  delay(100);   // go to sleep after 20 sec  
  
  if(millis() > 20000)  //10s mil stop
  {
  goSleep();
  }
    
  }
  else
  {
     Serial.println(" Waiting for Ble Client for 5sec ... [ Auto-connected save Wi-fi ]");
    if(notConnectedSince == 0)
    {
      //First time checking for bluetooth connection failed
      notConnectedSince = millis();//Save the current time for future reference
    }
    else if(millis() - notConnectedSince > 5*1000 && isWiFiCodeRunning == false)
    {
      //Glass not connected with bluetooth for 5 seconds
      //Therefore start WiFi Hotspot
      setupWiFi();
      isWiFiCodeRunning = true;
    }
    
  }
  
  if(isWiFiCodeRunning)loopWiFi();
  
  delay(100);
}




// DEEP SLEEP FUNCTION


void goSleep()
{
  Serial.println("DS5:.....");
  esp_deep_sleep_start();
  delay(100);
  Serial.println("SHOULD NOT PRINT");
}
