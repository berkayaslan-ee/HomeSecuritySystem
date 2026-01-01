
#include "HSS_EMBEDDED.h"

stHssManager_t hssManager;
EspSoftwareSerial::UART tftSerial;
Adafruit_MCP23X17 mcp;
RTC_DS1307 rtc;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup()
{
  hssManagerInit();

  Serial.begin(SLAVE_BAUDRATE);
  tftSerial.begin(TFT_BAUDRATE, EspSoftwareSerial::SWSERIAL_8N1, NEXTION_RX, NEXTION_TX);
  //tftChangePage( "page0" );
  //tftTextWrite( "p0tWelcome", "Welcome, System is Being Activated.." );
  delay(500);  // ekran açılma gecikmesi
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.status( ) != WL_CONNECTED ) 
  {
    delay( 500 );
  }

  //tftTextWrite("p0tWelcome", "Wi-Fi connection established.");
  //delay(3000);
  //tftTextWrite("p0tWelcome", "Home Security System is Active.");
  //delay(3000);
  //tftTextWrite("p0tWelcome", " ");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", ""))
  {
    signupOK = true;
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  mcp.begin_I2C();
  rtc.begin();
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  
  }

  pinMode(SLAVE_ENABLE_PIN, OUTPUT);
  pinMode(REED_SWITCH_PIN, INPUT);
  mcp.pinMode(LIVINGLED, OUTPUT);
  mcp.pinMode(KITCHENLED, OUTPUT);
  mcp.pinMode(BEDROOMLED, OUTPUT);
  digitalWrite(SLAVE_ENABLE_PIN, LOW);
  slaveDataGet();

  tftChangePage("page0");
  tftTextWrite("p0tWelcome", "System is Active.");
}

void loop() 
{  
  DateTime nowDeneme = rtc.now();
  sprintf(datetime, "%02d:%02d:%02d", nowDeneme.hour(), nowDeneme.minute(), nowDeneme.second());
  slaveDataReceiveTimeOut();
  alarmControl();
  tftDataRead();
  mcpDataSet();
  slaveDataRead();
  tftDataSet();
  firebaseSend(); 
  firebaseGet();
   
}

void alarmControl() 
{
  reedState =  digitalRead(REED_SWITCH_PIN);
  if (reedState) {
    if (hssManager.awayMode) {
      if (hssManager.passwordPageFlag) {
        tftChangePage("PinScreen");
        hssManager.passwordTimer = millis();
        hssManager.passwordPageFlag = 0;
      }
      if ((millis() - hssManager.passwordTimer) >= 1000) hssManager.passwordCount++;
      if (hssManager.passwordCount == 60) hssManager.mainReadSwitch = 1;
    } else hssManager.mainReadSwitch = 0;
  } else hssManager.mainReadSwitch = 0;

  if ((hssManager.awayMode) && (hssManager.slaveDatalivingRoom.slaveData.pirState == 0x01) || hssManager.slaveDataBedRoom.slaveData.pirState == 0x01 || hssManager.slaveDatakitchen.slaveData.pirState == 0x01 || (hssManager.slaveDatalivingRoom.slaveData.reedState == 0x01) || (hssManager.slaveDataBedRoom.slaveData.reedState == 0x01) || (hssManager.slaveDatakitchen.slaveData.reedState == 0x01) || hssManager.mainReadSwitch) hssManager.alarmState = 1;

  else if ((hssManager.slaveDatalivingRoom.slaveData.mq7State == 0x01) || (hssManager.slaveDataBedRoom.slaveData.mq7State == 0x01) || (hssManager.slaveDatakitchen.slaveData.mq7State == 0x01) || (hssManager.slaveDatalivingRoom.slaveData.panicState == 0x01) || (hssManager.slaveDataBedRoom.slaveData.panicState == 0x01) || (hssManager.slaveDatakitchen.slaveData.panicState == 0x01) || (hssManager.slaveDatakitchen.slaveData.rainState == 0x01)) hssManager.alarmState = 1;

  else hssManager.alarmState = 0;
}

void mcpDataSet() 
{
  if (hssManager.slaveDatalivingRoom.slaveData.pirState == 0x01 && !hssManager.awayMode) mcp.digitalWrite(LIVINGLED, HIGH);
  else mcp.digitalWrite(LIVINGLED, hssManager.livingRoomLed);
  if (hssManager.slaveDatakitchen.slaveData.pirState == 0x01 && !hssManager.awayMode) mcp.digitalWrite(KITCHENLED, HIGH);
  else mcp.digitalWrite(KITCHENLED, hssManager.kitchenLed); 
  if (hssManager.slaveDataBedRoom.slaveData.pirState == 0x01 && !hssManager.awayMode) mcp.digitalWrite(BEDROOMLED, HIGH);
  else mcp.digitalWrite(BEDROOMLED, hssManager.bedRoomLed);
}

void tftDataSet() 
{
  if ( !tftInitialColorSetDone )
  {
    tftInitialColorSet();
    tftInitialColorSetDone = true;
  }

  if( tftUpdateBed )
  {
    tftNumWrite("Bedroom.bedTempData", (int)(hssManager.slaveDataBedRoom.slaveData.temp * 10.0));
    tftNumWrite("Bedroom.bedHumData", (int)(hssManager.slaveDataBedRoom.slaveData.humid * 10.0));
    if (hssManager.slaveDataBedRoom.slaveData.mq7State == 0x01) tftTextWrite("Bedroom.bedMQ7Data", "Detected");
    else tftTextWrite("Bedroom.bedMQ7Data", "Not Detected");
    if (hssManager.slaveDataBedRoom.slaveData.pirState == 0x01) tftTextWrite("Bedroom.bedPirData", "Detected");
    else tftTextWrite("Bedroom.bedPirData", "Not Detected");
    if (hssManager.slaveDataBedRoom.slaveData.reedState == 0x01) tftTextWrite("Bedroom.bedWindowData", "Open");
    else tftTextWrite("Bedroom.bedWindowData", "Closed");
    if (hssManager.slaveDataBedRoom.slaveData.reedState == 0x01) tftTextWrite("Bedroom.bedWindowData", "Open");
    else tftTextWrite("Bedroom.bedWindowData", "Closed");
    if (hssManager.slaveDataBedRoom.slaveData.reedState == 0x01) tftTextWrite("Bedroom.bedWindowData", "Open");
    else tftTextWrite("Bedroom.bedWindowData", "Closed");
    if (hssManager.slaveDataBedRoom.slaveData.panicState == 0x01) tftTextWrite("Bedroom.bedPanicData", "Triggered");
    else tftTextWrite("Bedroom.bedPanicData", "Not Triggered");
  }

  if( tftUpdateKit )
  {
    tftNumWrite("Kitchen.kitTempData", (int)(hssManager.slaveDatakitchen.slaveData.temp * 10.0));
    tftNumWrite("Kitchen.kitHumData", (int)(hssManager.slaveDatakitchen.slaveData.humid * 10.0));
    if (hssManager.slaveDatakitchen.slaveData.mq7State == 0x01) tftTextWrite("Kitchen.kitMQ7Data", "Detected");
    else tftTextWrite("Kitchen.kitMQ7Data", "Not Detected");
    if (hssManager.slaveDatakitchen.slaveData.pirState == 0x01) tftTextWrite("Kitchen.kitPirData", "Detected");
    else tftTextWrite("Kitchen.kitPirData", "Not Detected");
    if (hssManager.slaveDatakitchen.slaveData.reedState == 0x01) tftTextWrite("Kitchen.kitWindowData", "Open");
    else tftTextWrite("Kitchen.kitWindowData", "Closed");
    if (hssManager.slaveDatakitchen.slaveData.panicState == 0x01) tftTextWrite("Kitchen.kitPanicData", "Triggered");
    else tftTextWrite("Kitchen.kitPanicData", "Not Triggered");
    if (hssManager.slaveDatakitchen.slaveData.rainState == 0x01) tftTextWrite("Kitchen.kitWaterData", "Yes");
    else tftTextWrite("Kitchen.kitWaterData", "No");
  }


  if( tftUpdateLiv )
  {
    tftNumWrite("LivingRoom.livTempData", (int)(hssManager.slaveDatalivingRoom.slaveData.temp * 10.0));
    tftNumWrite("LivingRoom.livHumData", (int)(hssManager.slaveDatalivingRoom.slaveData.humid * 10.0));
    if (hssManager.slaveDatalivingRoom.slaveData.mq7State == 0x01) tftTextWrite("LivingRoom.livMQ7Data", "Detected");
    else tftTextWrite("LivingRoom.livMQ7Data", "Not Detected");
    if (hssManager.slaveDatalivingRoom.slaveData.pirState == 0x01) tftTextWrite("LivingRoom.livPirData", "Detected");
    else tftTextWrite("LivingRoom.livPirData", "Not Detected");
    if (hssManager.slaveDatalivingRoom.slaveData.reedState == 0x01) tftTextWrite("LivingRoom.livWindowData", "Open");
    else tftTextWrite("LivingRoom.livWindowData", "Closed");
    if (hssManager.slaveDatalivingRoom.slaveData.panicState == 0x01) tftTextWrite("LivingRoom.livPanicData", "Triggered");
    else tftTextWrite("LivingRoom.livPanicData", "Not Triggered");
  }

  DateTime now = rtc.now();
  tftNumWrite("xHour", now.hour());
  tftNumWrite("xMinute", now.minute());
}

void tftDataRead() 
{
  if (tftSerial.available() >= 3) 
  {
    uint8_t startByteTft    = 0;
    uint8_t selectByteTft   = 0;
    uint8_t messageId       = 0;

    startByteTft = tftSerial.read();
    if (startByteTft == START_BYTE_TFT) 
    {
      selectByteTft = tftSerial.read();
      if (selectByteTft == SELECT_BYTE_TFT)
      {
        hssManager.tftButtonTrigger = 1;
        messageId = tftSerial.read();
        if (messageId == BEDROOM_MESSAGE_ID)
        {
          hssManager.bedRoomLed = !hssManager.bedRoomLed;
        } 
        else if (messageId == LIVINGROOM_MESSAGE_ID) 
        {
          hssManager.livingRoomLed = !hssManager.livingRoomLed;
        } 
        else if (messageId == KITCHEN_MESSAGE_ID) 
        {
          hssManager.kitchenLed = !hssManager.kitchenLed;
        } 
        else if (messageId == AWAY_MESSAGE_ID) 
        {
          hssManager.awayMode = !hssManager.awayMode;
        } 
        else if (messageId == PASSWORT_MESSAGE_ID) 
        {
          hssManager.awayMode = 0;
          hssManager.passwordCount = 0;
          hssManager.passwordPageFlag = 1;
          tftChangePage("page0");
        }
        tftColorSet();
      }
      else            hssManager.tftButtonTrigger = 0;
    }
    else
    {
        for(uint8_t i = 1; i < 3; ++i)
        {
          bufClean[i] = tftSerial.read();
        }
        hssManager.tftButtonTrigger = 0;
    }
  }
  else
  {
    tftNumWrite("startByte", 0);
    tftNumWrite("selectByte", 0);
    tftNumWrite("messageId", 0);
  }
}

void slaveDataRead() 
{
  if (Serial.available() >= sizeof(stSlaveData_t)) 
  {
    uint8_t startByte = 0;
    uint8_t messageId = 0;

    startByte = Serial.read();
    if (startByte == START_BYTE) 
    {
      messageId = Serial.read();
      
      if (messageId == BEDROOM_MESSAGE_ID) 
      {
        for (uint8_t i = 2; i < MAX_SLAVE_DATA_SIZE; i++) {
          hssManager.slaveDataBedRoom.slaveDataArray[i] = Serial.read();
        }
        tftUpdateBed = 1;
        firebaseUpdateBed = 1;
      } 
      else if (messageId == KITCHEN_MESSAGE_ID) 
      {
        for (uint8_t i = 2; i < MAX_SLAVE_DATA_SIZE; i++) {
          hssManager.slaveDatakitchen.slaveDataArray[i] = Serial.read();
        }
        tftUpdateKit = 1;
        firebaseUpdateKit = 1;
      } 
      else if (messageId == LIVINGROOM_MESSAGE_ID)
       {
        for (uint8_t i = 2; i < MAX_SLAVE_DATA_SIZE; i++) {
          hssManager.slaveDatalivingRoom.slaveDataArray[i] = Serial.read();
        }
        tftUpdateLiv = 1;
        firebaseUpdateLiv = 1;
      }
      slaveDataGet();
    }
    else
    {
        for (uint8_t i = 1; i < MAX_SLAVE_DATA_SIZE; i++) 
        {
          bufClean[i] = Serial.read();
        }
    }
  }
}

void slaveDataGet() 
{
  digitalWrite(SLAVE_ENABLE_PIN, HIGH);
  Serial.write(SLAVE_SELECT_START_BYTE);
  Serial.write(SLAVE_SELECT_MESSAGE_ID);
  Serial.write(hssManager.selectSlaveCount);
  Serial.write(uint8_t(hssManager.awayMode));
  delay(1);
  digitalWrite(SLAVE_ENABLE_PIN, LOW);

  hssManager.slaveReceiveTimer = millis();

  hssManager.selectSlaveCount++;

  if (hssManager.selectSlaveCount == 36) hssManager.selectSlaveCount = 33;
}

void hssManagerInit() {
  hssManager.selectSlaveCount = 0x21;
  hssManager.timeOutCount = 0;
  hssManager.bedRoomLed = 0;
  hssManager.livingRoomLed = 0;
  hssManager.kitchenLed = 0;
  hssManager.awayMode = 0;
  hssManager.alarmState = 0;
  hssManager.passwordCount = 0;
  hssManager.passwordPageFlag = 1;

  for (uint8_t i = 0; i < 19; i++) {
    hssManager.slaveDataBedRoom.slaveDataArray[i] = 0;
    hssManager.slaveDatalivingRoom.slaveDataArray[i] = 0;
    hssManager.slaveDatakitchen.slaveDataArray[i] = 0;
  }
}

void slaveDataReceiveTimeOut() 
{
  if ((millis() - hssManager.slaveReceiveTimer) > 100) 
  {
        digitalWrite(SLAVE_ENABLE_PIN, HIGH);
        Serial.write(SLAVE_SELECT_START_BYTE);
        Serial.write(SLAVE_SELECT_MESSAGE_ID);
        Serial.write(hssManager.selectSlaveCount);
        Serial.write(uint8_t(hssManager.awayMode));
        delay(1);
        digitalWrite(SLAVE_ENABLE_PIN, LOW);
    hssManager.slaveReceiveTimer = millis();
    //hssManager.timeOutCount++;
    //if( hssManager.timeOutCount == 3 )
   // {
        if (hssManager.selectSlaveCount == 0x21) hssManager.selectSlaveCount = 0x22;
        else if (hssManager.selectSlaveCount == 0x22) hssManager.selectSlaveCount = 0x23;
        else if (hssManager.selectSlaveCount == 0x23) hssManager.selectSlaveCount = 0x21;
        //hssManager.timeOutCount = 0;
   // }
  }
}

void firebaseSend()
{
  DateTime nowRtc = rtc.now(); 
  String mainDoorStatus;
  char datetime[20];
  sprintf(datetime, "%02d:%02d:%02d", nowRtc.hour(), nowRtc.minute(), nowRtc.second());
  if(Firebase.ready() && signupOK )
  {
    if ( anyChangeBedroom() )
    {
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Alarm", hssManager.alarmState );
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Gas", hssManager.slaveDataBedRoom.slaveData.mq7State);
      Firebase.RTDB.setBool(&fbdo, "bedroom/lastPacket/LED", hssManager.bedRoomLed);
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/PIR", hssManager.slaveDataBedRoom.slaveData.pirState);
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Temperature", hssManager.slaveDataBedRoom.slaveData.temp);
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Humidity", hssManager.slaveDataBedRoom.slaveData.humid);
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Panic", hssManager.slaveDataBedRoom.slaveData.panicState);
      Firebase.RTDB.setInt(&fbdo, "bedroom/lastPacket/Reed", hssManager.slaveDataBedRoom.slaveData.reedState);
      Firebase.RTDB.setString(&fbdo, "bedroom/lastPacket/time", datetime);
    }

    if (anyChangeKitchen())
    {
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Alarm", hssManager.alarmState);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Gas", hssManager.slaveDatakitchen.slaveData.mq7State);
      Firebase.RTDB.setBool(&fbdo, "kitchen/lastPacket/LED", hssManager.kitchenLed);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/PIR", hssManager.slaveDatakitchen.slaveData.pirState);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Temperature", hssManager.slaveDatakitchen.slaveData.temp);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Humidity", hssManager.slaveDatakitchen.slaveData.humid);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Panic", hssManager.slaveDatakitchen.slaveData.panicState);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Reed", hssManager.slaveDatakitchen.slaveData.reedState);
      Firebase.RTDB.setInt(&fbdo, "kitchen/lastPacket/Rain", hssManager.slaveDatakitchen.slaveData.rainState);
      Firebase.RTDB.setString(&fbdo, "kitchen/lastPacket/time", datetime);
    }

    if (anyChangeLivingRoom())
    {
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Alarm", hssManager.alarmState);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Gas", hssManager.slaveDatalivingRoom.slaveData.mq7State);
      Firebase.RTDB.setBool(&fbdo, "living_room/lastPacket/LED", hssManager.livingRoomLed);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/PIR", hssManager.slaveDatalivingRoom.slaveData.pirState);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Temperature", hssManager.slaveDatalivingRoom.slaveData.temp);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Humidity", hssManager.slaveDatalivingRoom.slaveData.humid);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Panic", hssManager.slaveDatalivingRoom.slaveData.panicState);
      Firebase.RTDB.setInt(&fbdo, "living_room/lastPacket/Reed", hssManager.slaveDatalivingRoom.slaveData.reedState);
      Firebase.RTDB.setString(&fbdo, "living_room/lastPacket/time", datetime);
    }

    if ( hssManager.tftButtonTrigger || hssManager.firebaseReceiveFlag )
    {
      awayModeEsp = ( hssManager.awayMode ? "Away" : "Home" );
      Firebase.RTDB.setString(&fbdo, "Mode/ESPMode", awayModeEsp);
      Firebase.RTDB.setString(&fbdo, "Mode/AndroidMode", awayModeEsp);
      Firebase.RTDB.setBool(&fbdo, "bedroom/lastPacket/LED", hssManager.bedRoomLed );
      Firebase.RTDB.setBool(&fbdo, "kitchen/lastPacket/LED", hssManager.kitchenLed );
      Firebase.RTDB.setBool(&fbdo, "living_room/lastPacket/LED", hssManager.livingRoomLed );
      Firebase.RTDB.setInt(&fbdo, "A_Change", 0);
      hssManager.firebaseReceiveFlag = 0;
      hssManager.tftButtonTrigger = 0;
    }
    
    mainDoorStatus = (digitalRead(REED_SWITCH_PIN) ? "Open" : "Closed");
    Firebase.RTDB.setString(&fbdo, "MainDoor/MainDoorStatus", mainDoorStatus);
  }
}

void firebaseGet()
{
  if( millis() - lastFirebaseReceiveTime >= 3000 )
  {
    Firebase.RTDB.getInt(&fbdo, "A_Change", &hssManager.androidChange);
    if( hssManager.androidChange )
    {
      Firebase.RTDB.getBool(&fbdo, "A_LED/b_LED", &hssManager.androidBedroomLed);
      Firebase.RTDB.getBool(&fbdo, "A_LED/k_LED", &hssManager.androidKitchenLed);
      Firebase.RTDB.getBool(&fbdo, "A_LED/l_LED", &hssManager.androidLivingLed);
      Firebase.RTDB.getString(&fbdo, "Mode/AndroidMode", &hssManager.androidAway);
      hssManager.bedRoomLed = hssManager.androidBedroomLed;
      hssManager.kitchenLed = hssManager.androidKitchenLed;
      hssManager.livingRoomLed = hssManager.androidLivingLed;
        
      hssManager.awayMode = ( hssManager.androidAway == "Away" ) ? true : false;
      hssManager.firebaseReceiveFlag = 1;
      tftColorSet();
      firebaseSend();
      lastFirebaseReceiveTime = millis();
    }
  }
}

bool anyChangeBedroom()
{
  bool changed = false;

  if (hssManager.alarmState != bedroomLastAlarmState) {
    bedroomLastAlarmState = hssManager.alarmState;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.mq7State != bedroomLastMQ7State) {
    bedroomLastMQ7State = hssManager.slaveDataBedRoom.slaveData.mq7State;
    changed = true;
  }

  if (hssManager.bedRoomLed != bedroomLastLEDState) {
    bedroomLastLEDState = hssManager.bedRoomLed;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.pirState != bedroomLastPirState) {
    bedroomLastPirState = hssManager.slaveDataBedRoom.slaveData.pirState;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.temp != bedroomLastTemp) {
    bedroomLastTemp = hssManager.slaveDataBedRoom.slaveData.temp;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.humid != bedroomLastHumid) {
    bedroomLastHumid = hssManager.slaveDataBedRoom.slaveData.humid;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.panicState != bedroomLastPanicState) {
    bedroomLastPanicState = hssManager.slaveDataBedRoom.slaveData.panicState;
    changed = true;
  }

  if (hssManager.slaveDataBedRoom.slaveData.reedState != bedroomLastReedState) {
    bedroomLastReedState = hssManager.slaveDataBedRoom.slaveData.reedState;
    changed = true;
  }

  return changed;
}

bool anyChangeKitchen()
{
  bool changed = false;

  if (hssManager.alarmState != kitchenLastAlarmState) {
    kitchenLastAlarmState = hssManager.alarmState;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.mq7State != kitchenLastMQ7State) {
    kitchenLastMQ7State = hssManager.slaveDatakitchen.slaveData.mq7State;
    changed = true;
  }

  if (hssManager.kitchenLed != kitchenLastLEDState) {
    kitchenLastLEDState = hssManager.kitchenLed;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.pirState != kitchenLastPirState) {
    kitchenLastPirState = hssManager.slaveDatakitchen.slaveData.pirState;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.temp != kitchenLastTemp) {
    kitchenLastTemp = hssManager.slaveDatakitchen.slaveData.temp;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.humid != kitchenLastHumid) {
    kitchenLastHumid = hssManager.slaveDatakitchen.slaveData.humid;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.panicState != kitchenLastPanicState) {
    kitchenLastPanicState = hssManager.slaveDatakitchen.slaveData.panicState;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.reedState != kitchenLastReedState) {
    kitchenLastReedState = hssManager.slaveDatakitchen.slaveData.reedState;
    changed = true;
  }

  if (hssManager.slaveDatakitchen.slaveData.rainState != kitchenLastRainState) {
    kitchenLastRainState = hssManager.slaveDatakitchen.slaveData.rainState;
    changed = true;
  }

  return changed;
}

bool anyChangeLivingRoom()
{
  bool changed = false;

  if (hssManager.alarmState != living_roomLastAlarmState) {
    living_roomLastAlarmState = hssManager.alarmState;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.mq7State != living_roomLastMQ7State) {
    living_roomLastMQ7State = hssManager.slaveDatalivingRoom.slaveData.mq7State;
    changed = true;
  }

  if (hssManager.livingRoomLed != living_roomLastLEDState) {
    living_roomLastLEDState = hssManager.livingRoomLed;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.pirState != living_roomLastPirState) {
    living_roomLastPirState = hssManager.slaveDatalivingRoom.slaveData.pirState;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.temp != living_roomLastTemp) {
    living_roomLastTemp = hssManager.slaveDatalivingRoom.slaveData.temp;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.humid != living_roomLastHumid) {
    living_roomLastHumid = hssManager.slaveDatalivingRoom.slaveData.humid;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.panicState != living_roomLastPanicState) {
    living_roomLastPanicState = hssManager.slaveDatalivingRoom.slaveData.panicState;
    changed = true;
  }

  if (hssManager.slaveDatalivingRoom.slaveData.reedState != living_roomLastReedState) {
    living_roomLastReedState = hssManager.slaveDatalivingRoom.slaveData.reedState;
    changed = true;
  }

  return changed;
}

void tftChangeThreeBco( String obj1, String obj2, String obj3, uint8_t num )
{
  tftSerial.print(obj1 + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);

  tftSerial.print(obj2 + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);

  tftSerial.print(obj3 + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

void tftChangeTwoBco( String obj1, String obj2, uint8_t num )
{
  tftSerial.print(obj1 + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);

  tftSerial.print(obj2 + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

void tftInitialColorSet()
{
  if ( hssManager.awayMode ) {
    tftChangeBco("HomePage.bMode", RED);
    tftTextWrite("HomePage.txtMode", "Mode: Away");
  } else {
    tftChangeBco("HomePage.bMode", GREEN);
    tftTextWrite("HomePage.txtMode", "Mode: Home");
  }

  if( hssManager.bedRoomLed ) {
    tftChangeBco( "Bedroom.bedLed", GREEN );
    tftTextWrite( "Bedroom.bedLed", "Turn off LED");
    tftTextWrite( "Bedroom.txtBedLed", "The LED is ON" );
  } else {
    tftChangeBco( "Bedroom.bedLed", RED );
    tftTextWrite( "Bedroom.bedLed", "Turn on LED");
    tftTextWrite( "Bedroom.txtBedLed", "The LED is OFF" );
  }

  if( hssManager.kitchenLed ) {
    tftChangeBco( "Kitchen.kitLed", GREEN );
    tftTextWrite( "Kitchen.kitLed", "Turn off LED");
    tftTextWrite( "Kitchen.txtKitLed", "The LED is ON" );
  } else {
    tftChangeBco( "Kitchen.kitLed", RED );
    tftTextWrite( "Kitchen.kitLed", "Turn on LED");
    tftTextWrite( "Kitchen.txtKitLed", "The LED is OFF" );
  }

  if( hssManager.livingRoomLed ) {
    tftChangeBco( "LivingRoom.livLed", GREEN );
    tftTextWrite( "LivingRoom.livLed", "Turn off LED");
    tftTextWrite( "LivingRoom.txtLivLed", "The LED is ON" );
  } else {
    tftChangeBco( "LivingRoom.livLed", RED );
    tftTextWrite( "LivingRoom.livLed", "Turn on LED");
    tftTextWrite( "LivingRoom.txtLivLed", "The LED is OFF" );
  }
}


void tftColorSet()
{
  if ( hssManager.firebaseReceiveFlag || hssManager.tftButtonTrigger )
  {
    tftChangeBco( "HomePage.bMode", hssManager.awayMode ? RED : GREEN);
    tftTextWrite( "HomePage.bMode", hssManager.awayMode ? "Activate Home" : "Activate Away" );
    tftTextWrite( "HomePage.txtMode", hssManager.awayMode ? "Mode: Away" : "Mode: Home" );

    tftChangeBco( "Bedroom.bedLed", hssManager.bedRoomLed ? GREEN : RED );
    tftTextWrite( "Bedroom.txtBedLed", hssManager.bedRoomLed ? "LED is ON" : "LED is OFF");
    tftTextWrite( "Bedroom.bedLed", hssManager.bedRoomLed ? "Turn Off LED" : "Turn On LED");

    tftChangeBco( "LivingRoom.livLed", hssManager.livingRoomLed ? GREEN : RED );
    tftTextWrite( "LivingRoom.txtLivLed", hssManager.livingRoomLed ? "LED is ON" : "LED is OFF");
    tftTextWrite( "LivingRoom.livLed", hssManager.livingRoomLed ? "Turn Off LED" : "Turn On LED");

    tftChangeBco( "Kitchen.kitLed", hssManager.kitchenLed ? GREEN : RED );
    tftTextWrite( "Kitchen.txtKitLed", hssManager.kitchenLed ? "LED is ON" : "LED is OFF");
    tftTextWrite( "Kitchen.kitLed", hssManager.kitchenLed ? "Turn Off LED" : "Turn On LED");
  }
} 


void tftNumWrite(String obj, uint16_t num) {
  tftSerial.print(obj + ".val=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

void tftTextWrite(String obj, String data) {
  tftSerial.print(obj + ".txt=\"" + data + "\"");
  //tftSerial.print( num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

void tftChangeBco(String obj, uint16_t num) {
  tftSerial.print(obj + ".bco=");
  tftSerial.print(num);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

void tftChangePage(String pageName) {
  tftSerial.print("page " + pageName);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
  tftSerial.write(0xFF);
}

