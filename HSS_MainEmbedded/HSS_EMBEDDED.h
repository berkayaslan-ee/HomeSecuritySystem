  #include <stdint.h>
  #include <SoftwareSerial.h>
  #include <Adafruit_MCP23X17.h>
  #include <RTClib.h>  
  #include <WiFi.h>
  #include <Firebase_ESP_Client.h>
  #include "addons/TokenHelper.h"
  #include "addons/RTDBHelper.h"

  char datetime[20];
  #define WIFI_SSID "Berkay"
  #define WIFI_PASSWORD "gfax5252*"

 // #define WIFI_SSID "Aslan"
  //#define WIFI_PASSWORD "06aslan211*"
  #define API_KEY "AIzaSyB_jk9WXN9JKpVy-49KzkUIF_6cCQ_5ulc"
  #define DATABASE_URL "https://deneme-a8609-default-rtdb.europe-west1.firebasedatabase.app/ "
  uint32_t sendDataPrevMillis = 0;
  bool signupOK = false;

  #define START_BYTE_SIZE       0
  #define MESSAGE_ID_BYTE_SIZE  1

  #define START_BYTE            0xA5

  #define START_BYTE_TFT        0x30
  #define SELECT_BYTE_TFT       0x40
  #define BEDROOM_MESSAGE_ID    0x21
  #define KITCHEN_MESSAGE_ID    0x22
  #define LIVINGROOM_MESSAGE_ID 0x23
  #define AWAY_MESSAGE_ID       0x24
  #define PASSWORT_MESSAGE_ID   0x25
  #define MAX_SLAVE_DATA_SIZE   15

  #define LIVINGLED                   0
  #define KITCHENLED                  1
  #define BEDROOMLED                  2
  #define REED_SWITCH_PIN             5

  #define RED                         47104
  #define GREEN                       1024
  #define WHITE                       65535
  #define BLACK                       0

  #define SLAVE_SELECT_START_BYTE 0x25
  #define SLAVE_SELECT_MESSAGE_ID 0x50

  #define SLAVE_BAUDRATE        115200
  #define TFT_BAUDRATE          9600

  #define NEXTION_RX                  16
  #define NEXTION_TX                  17

  #define SLAVE_ENABLE_PIN      4


  /*Slave Data Set*/
  typedef struct stSlaveData_t
  {
    uint8_t startByte;
    uint8_t messageId;
    uint8_t reedState;
    uint8_t pirState;
    uint8_t panicState;
    uint8_t rainState;
    uint8_t mq7State;
    uint8_t   temp;
    uint8_t   humid;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
  };

  typedef union uSlaveData_t
  {
    stSlaveData_t slaveData;
    uint8_t slaveDataArray[ sizeof( stSlaveData_t ) ];
  };

  /*TFT Data Set*/
  typedef struct stTftData_t
  {
    uint8_t startByte;
    uint8_t messageId;
    uint8_t command;
  };

  typedef union uTftData_t
  {
    stTftData_t tftData;
    uint8_t tftDataArray[ sizeof( stTftData_t ) ];
  };

  typedef struct stHssManager_t
  {
    union uSlaveData_t  slaveDataBedRoom;
    union uSlaveData_t  slaveDatakitchen;
    union uSlaveData_t  slaveDatalivingRoom;
    union uTftData_t    tftData;

    uint8_t timeOutCount;
    uint8_t       selectSlaveCount;
    bool livingRoomLed;
    bool kitchenLed;
    bool bedRoomLed;
    bool awayMode;
    bool alarmState;
    bool mainReadSwitch;
    uint32_t passwordTimer;
    uint32_t slaveReceiveTimer;
    uint8_t passwordCount;
    bool passwordTimerEnable; 
    bool passwordPageFlag;

    bool androidLivingLed;
    bool androidKitchenLed;
    bool androidBedroomLed;
    String androidAway;
    uint8_t androidChange;
    uint8_t tftButtonTrigger;
    uint8_t firebaseReceiveFlag;

  };
  bool tftUpdateLiv = 0;
  bool tftUpdateBed = 0;
  bool tftUpdateKit = 0;

  bool firebaseUpdateBed = 0;
  bool firebaseUpdateKit = 0;
  bool firebaseUpdateLiv = 0;
  bool androidDataChangeFlag = 0;

  bool reedState = 0;

  uint32_t lastFirebaseReceiveTime = 0;

  uint8_t bedroomLastAlarmState = -1;
  uint8_t bedroomLastMQ7State = -1;
  uint8_t bedroomLastLEDState = -1;
  uint8_t bedroomLastPirState = -1;
  uint8_t bedroomLastTemp = -1;
  uint8_t bedroomLastHumid = -1;
  uint8_t bedroomLastPanicState = -1;
  uint8_t bedroomLastReedState = -1;
  String bedroomLastDateTime = "00:00:00";

  uint8_t kitchenLastAlarmState = -1;
  uint8_t kitchenLastMQ7State = -1;
  uint8_t kitchenLastLEDState = -1;
  uint8_t kitchenLastPirState = -1;
  uint8_t kitchenLastTemp = -1;
  uint8_t kitchenLastHumid = -1;
  uint8_t kitchenLastPanicState = -1;
  uint8_t kitchenLastReedState = -1;
  uint8_t kitchenLastRainState = -1;
  String kitchenLastDateTime = "00:00:00";

  uint8_t living_roomLastAlarmState = -1;
  uint8_t living_roomLastMQ7State = -1;
  uint8_t living_roomLastLEDState = -1;
  uint8_t living_roomLastPirState = -1;
  uint8_t living_roomLastTemp = -1;
  uint8_t living_roomLastHumid = -1;
  uint8_t living_roomLastPanicState = -1;
  uint8_t living_roomLastReedState = -1;
  String living_roomLastDateTime = "00:00:00";
  String awayModeEsp;
  bool tftInitialColorSetDone = false;

  uint8_t bufClean[MAX_SLAVE_DATA_SIZE];
  void slaveDataGet();
  void hssManagerInit();
  void slaveDataRead();
  void tftDataRead();
  void mcpDataSet();
  void tftChangePage( String pageName );
  void tftChangeBco( String obj, uint16_t num );
  void tftTextWrite( String obj, String data );
  void tftNumWrite( String obj, uint16_t num );
  void tftChangePage( String pageName );
  void alarmControl();
  void slaveDataReceiveTimeOut();
  void firebaseSend();
  void firebaseGet();
  void tftInitialColorSet();
  void tftColorSet();
  bool anyChangeBedroom();



