/****************************************************************************
 ***************************** Header File List *****************************
 ***************************************************************************/
#include<otadrive_esp.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include<EEPROM.h>


/****************************************************************************
 ************** Wifi Connectivity and Update Functions List *****************
 ***************************************************************************/
void connectWiFiWithEEP(bool t);
void uploadwiFiCrToEEP();
void reloadwiFiCrFromEEP();
void upgrade();
void OnProgress(int progress, int total);


/****************************************************************************
 *********  Data Processing and Task Handelling Functions List **************
 ***************************************************************************/
void Localtask();
void Cloudtask(bool ConnectCloud);
void process_engine(String s);
void smart_mode(bool state);
void safe_mode(bool state);


/****************************************************************************
 ***************************** Sensors Function List ************************
 ***************************************************************************/
double readUltrasonicDistance(const int trigger,const int echo);
bool is_Anyone_inside(const int trig, const int echo);
bool is_Weather_Clear(const int powerpin,const int datapin);
void alert(const int buz);
bool is_Fire_Detected(const int fr);


/****************************************************************************
 ********************************* Variable List ****************************
 ***************************************************************************/
#define FIREBASE_API_KEY "5ZVeS9mak39X9mCV10ujPAspgJPGWSv76mfjp6uu"
#define DATABASE_URL "homeautomation-c8173-default-rtdb.firebaseio.com"
#define ReqAccess "4iroX8Xjc53K1NdoOhMZb4bfIFM7u915fVmn0TgS"
#define ReqAccess_URL "dream-palace-37cf0-default-rtdb.firebaseio.com"

int num_of_dev=2;
String validity="";
String Dev[2]={"led1","fan1"};
String SSID1 = "Ruhi";
String WIFIPASS = "nadia8899";
String OTA_api_key = "8927a03d-b29e-4ffc-b45f-4359060e7328", version = "1.0.0.1";
String Data = "", FDB = "",prev_fdb="";
String con="";
unsigned long DataMillis = 0,wifiMillis=0,powerMillis=0;
int count = 0,i=0;
bool ConnectCloud=false;
bool SmartMode=false,SafeMode=false;
bool ON = HIGH;
bool OFF = LOW;
bool beganfrbs=false;
bool authenticfirebase=false;

/****************************************************************************
 ********************************* Object List ******************************
 ***************************************************************************/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


/*****************************************************************************************************
************************************ Defining Pin Number *********************************************
******************************************************************************************************
*/

//GND
#define RoomtriggerPin 33
#define RoomechoPin 32
//VCC

//GND
#define RainData 5
#define RainPower 4  //FAKE VCC:

//GND
#define led1 18

//GND
#define fan1 19
//VCC

//GND
#define buz 22

//GND
#define fire 14
//VCC

/*
****************************************
****************************************
****************************************
*/




void setup() {
  Serial.begin(9600);
  Serial.println("Started");
  EEPROM.begin(512); //Initializing EEPROM
  pinMode(RoomtriggerPin,OUTPUT);
  pinMode(RoomechoPin,INPUT);
  pinMode(fire,INPUT);
  pinMode(led1,OUTPUT);
  pinMode(fan1,OUTPUT);
  pinMode(buz,OUTPUT);
  delay(5000);
  Serial.println("V-" + version);
  OTADRIVE.setInfo(OTA_api_key, version);
  OTADRIVE.onUpdateFirmwareProgress(OnProgress);
  Request_Access();
  if(authenticfirebase)ConnectCloud=true;
}


void loop() {
  if(!is_Weather_Clear(RainPower,RainData))alert(true,1000,"Raining Alert .....");
  if(is_Fire_Detected(fire))alert(true,1000,"Fire Alert!");
  if(beganfrbs==false&&WiFi.status() == WL_CONNECTED){
    beganfrbs==true;
    config.database_url = DATABASE_URL;
    config.signer.test_mode = true;
    Firebase.begin(&config, &auth);
    }
  if (millis() - wifiMillis > 1000 || wifiMillis == 0){
    wifiMillis=millis();
    if(WiFi.status() == WL_CONNECTED){con="CE";}
    else {con="CL";}
    
    }
  
  Cloudtask(ConnectCloud);
  Localtask();   
  if (millis() - powerMillis > 1000 || wifiMillis == 0){
  powerMillis=millis();
  Serial.println("!wifi-"+con+":cloud-"+(String)ConnectCloud+":Room-"+(String)is_Anyone_inside(33,32)+":Weather-"+(String)is_Weather_Clear(4,5)+":Fire-"+(String)is_Fire_Detected(14)+":Distance-"+(String)readUltrasonicDistance(RoomtriggerPin,RoomechoPin));
  }
  smart_mode(SmartMode);
}







void Cloudtask(bool ConnectCloud) {
  if (ConnectCloud&&authenticfirebase) {
    if (Firebase.ready() && (millis() - DataMillis > 1000 || DataMillis == 0)) {
      DataMillis = millis();
      FDB = Firebase.getString(fbdo, "/test/tag") ? fbdo.to<const char *>() : fbdo.errorReason().c_str();
      FDB.remove(FDB.length() - 3, 3);
      FDB.remove(0, 2);
      Serial.println(FDB);
      if (prev_fdb.compareTo(FDB) != 0) {
        process_engine(FDB);
      }
      prev_fdb = FDB;
      FDB = "";
    }
  }
}



void Localtask() {
  if (Serial.available() > 0) {
    Data = Serial.readString();
    Serial.println(Data.compareTo("led1off"));
    process_engine(Data);
    Data = "";
  }
}


void process_engine(String s) {
  if (s[0] == '?' && s[1] == 'S' && s[2] == 'P') {
    uploadwiFiCrToEEP(s);
    return;
  }
  if (s.compareTo("update") == 0||s.compareTo("update") == 10) upgrade();
  else if (s.compareTo("wifioff") == 0||s.compareTo("wifioff") == 10)WiFi.mode(WIFI_OFF);
  else if (s.compareTo("connect") == 0||s.compareTo("connect") == 10)connectWiFiWithEEP(false);
  else if (s.compareTo("connecteep") == 0||s.compareTo("connecteep") == 10)connectWiFiWithEEP(true);
  else if (s.compareTo("smartmodeon") == 0||s.compareTo("smartmodeon") == 10)SmartMode=true;
  else if (s.compareTo("smartmodeoff") == 0||s.compareTo("smartmodeoff") == 10)SmartMode=false;
  else if (s.compareTo("ConnectCloud") == 0||s.compareTo("ConnectCloud") == 10)ConnectCloud=true;
  else if (s.compareTo("disConnectCloud") == 0||s.compareTo("disConnectCloud") == 10)ConnectCloud=false; 
  else if (s.compareTo("led1on") == 0||s.compareTo("led1on") == 10){digitalWrite(led1,ON);}
  else if (s.compareTo("led1off") == 0||s.compareTo("led1off") == 10){digitalWrite(led1,OFF);}
  else if (s.compareTo("fan1on") == 0||s.compareTo("fan1on") ==10){digitalWrite(fan1,ON);}
  else if (s.compareTo("fan1off") == 0||s.compareTo("fan1off") == 10){digitalWrite(fan1,OFF);}
} 








void connectWiFiWithEEP(bool t) {
  if (t) {
    reloadwiFiCrFromEEP();
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID1.c_str(), WIFIPASS.c_str());
    Serial.println("Trying to connect ssid:" + SSID1 + " PASS:" + WIFIPASS);
    byte tried = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(250);
      tried++;
      if (tried == 20) {
        tried = 0;
        Serial.println("Coudn't connect!");
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("Connected to " + SSID1);
    }
  }
  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin("Ruhi" , "nadia8899");
    byte tried = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(250);
      tried++;
      if (tried == 20) {
        tried = 0;
        Serial.println("Coudn't connect!");
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("Connected to wifi");
    }

  }
}

void uploadwiFiCrToEEP(String DATA) {
  SSID1 = "";
  WIFIPASS = "";
  bool ssidfound = false, passfound = false;
  for (int i = 3; i < DATA.length(); i++) {
    if (ssidfound && DATA[i] == '|')passfound = true;
    if (passfound)break;
    if (DATA[i] == '~') {
      ssidfound = true;
      continue;
    }
    if (!ssidfound)
      SSID1 += DATA[i];
    if (ssidfound && !passfound) {
      WIFIPASS += DATA[i];
    }
  }

  if (SSID1.length() > 0 && WIFIPASS.length() > 0) {
    //WIFIPASS.remove(WIFIPASS.length()-2,2);
    EEPROM.write(90, SSID1.length()); EEPROM.write(100, WIFIPASS.length());
    for (int i = 0; i < 65; ++i)EEPROM.write(i, 0);
    for (int i = 0; i < SSID1.length(); ++i)EEPROM.write(i, SSID1[i]);
    for (int i = 0; i < WIFIPASS.length(); ++i)EEPROM.write(32 + i, WIFIPASS[i]);
    EEPROM.commit();
    Serial.println("UPLoaded wifi credential to eep ssid:" + SSID1 + " pass:" + WIFIPASS);
    Serial.println(WIFIPASS.length());

  }
}



void reloadwiFiCrFromEEP() {
  SSID1 = "";
  WIFIPASS = "";
  for (int i = 0; i < EEPROM.read(90); ++i)SSID1 += char(EEPROM.read(i));
  for (int i = 32; i < 32 + EEPROM.read(100); ++i)WIFIPASS += char(EEPROM.read(i));
  Serial.println("Loaded wifi credential from eep ssid:" + SSID1 + " pass:" + WIFIPASS);
}




void upgrade() {
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Request For Update!");
    auto inf = OTADRIVE.updateFirmwareInfo();
    if (inf.available)
    {
      Serial.printf("UV%s", inf.version.c_str());
      Serial.println("");
      OTADRIVE.updateFirmware();
    }
    else
      Serial.println("No update available!");
  }
  else
    Serial.println("Sorry device is offline! Please connect to the internet...");

}

void OnProgress(int progress, int total)
{
  static int last_percent = 0;
  int percent = (100 * progress) / total;
  if (percent != last_percent)
  {
    Serial.println("U"+(String)percent+"%");
    last_percent = percent;
  }
}

void Request_Access(){
  connectWiFiWithEEP(false);
  if (WiFi.status() == WL_CONNECTED) {
  Serial.println("Requesting for access....");  
  }
  config.database_url = ReqAccess_URL;
  config.signer.test_mode = true;
  Firebase.begin(&config,&auth);
  while(true){
  validity = Firebase.getString(fbdo,"/test/validity") ? fbdo.to<const char *>() : fbdo.errorReason().c_str();
  validity.remove(validity.length()-2,2);
  validity.remove(0,2);
  Serial.println(validity);
  if(validity.length()>0)break;
  }
  while(true){
  if(validity.compareTo("J.A.R.V.I.S. by ZeroHz") == 0){authenticfirebase=true,Serial.println("Access Granted");break;} 
  else{Serial.println("Access denied.");delay(5000);ESP.restart();}
  } 
}


//Return distance
double readUltrasonicDistance(const int trigger,const int echo)
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  return 0.01723 * pulseIn(echo, HIGH);
}


//Return true if anyone is inside the room
bool is_Anyone_inside(const int trig, const int echo) {
  if (readUltrasonicDistance(trig, echo) < 28) {
    return true;
  }
  else {
    return false;
  }

}


bool is_Weather_Clear(const int powerpin, const int datapin) {
  int val = digitalRead(datapin); // Read the sensor output
  if(val==1)return true;
  else return false;
}

bool is_Fire_Detected(const int fr){
  if(digitalRead(fr)==0)return true;
  else if(digitalRead(fr)==0)return false;
  else return !digitalRead(fr);
  }


void alert(bool a,int t,String msg) {
  if(a){
    digitalWrite(buz,HIGH);
  Serial.println(msg);
  delay(t);
  digitalWrite(buz,LOW);
  
  }
}

void smart_mode(bool state){
  if(state){
    if(!is_Fire_Detected(fire)){
    if(!is_Anyone_inside(RoomtriggerPin,RoomechoPin)){
      digitalWrite(fan1,OFF);
      digitalWrite(led1,OFF);
     }
     else if(is_Weather_Clear(RainPower,RainData)){
     digitalWrite(fan1,ON);
     digitalWrite(led1,ON);
      }
     else if(!is_Weather_Clear(RainPower,RainData)){
      digitalWrite(fan1,OFF);
      digitalWrite(led1,ON);
      }
      }
      else {Serial.println("Fire Alert!");digitalWrite(fan1,OFF);
      digitalWrite(led1,OFF);alert(true,1000,"Fire Alert!");} 
    }
  }
