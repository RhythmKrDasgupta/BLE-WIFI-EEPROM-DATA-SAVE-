# BLE-WIFI-EEPROM-DATA-SAVE-
BLE SWITCH TO WIFI CLIENT MODE BLE SNT WIFI USER NAME AND PASSWORD            
Waitng For Bluetooth client ( for 5 secound). Bluetooth client Application Setup wifi Credentials in EEPROM  Bletooth can communicate with this device. When Bluetooth client is not connected for 10 secound , Device will auto connected to the wifi network (ssid ,password already saved in eeprom)       Deviced will communicate with ip address when it wifi range    

     1. DEEP SLEEP AFTER SUCCESFULL DATA SENDING THROUGH BLE     
     2. GPIO 33 PULL HIGH to WAKEUP AND COMMUNICXATE OVER BLE     
     3. IF BLE CLIENT DNT CONNECT FOR 10 Sec AUTO CONNECT TO WIFI WHICH ID , PASS SEND BY BLE CLIENT       
     4. EEPROM WIFI &amp; PASS SAVE      
     5. WHEN BLE IS CONNECTED WIFI CLIENT WILL OFF AND VISEVARSA     
     6. Read ADC Battery Voltage &amp; Tranismit it Through BLE And Webserver       
     7. BUZZER PLAY for 1 sec when it tigger     KEYWORD : EEPROM , ESP32 , DEEP_SLEEP , TOUCH_WAKEUP , RTC_WAKEUP ,Bluetooth_LOW_ENERGY,        ESP32_SERVER , ADC (Battery voltage);     code update : 27 OCT 18
