// ESP32-WROOM-32; Arduino ESP32 core 2.0.17; IRremote 4.4.1;
// ESP32 BLE Keyboard 0.3.2-beta (T-vK). Do not enable USE_NIMBLE.
#include <Arduino.h>
#include <Preferences.h>
#include <BleKeyboard.h>
#include <IRremote.hpp>
#include "Logic.h"
constexpr uint8_t IR_PIN=27;
constexpr int KEY_COUNT=14;
const char* labels[KEY_COUNT]={"NEXT", "PREVIOUS", "PLAY / START", "BACK / EXIT",
 "0","1","2","3","4","5","6","7","8","9"};
struct Binding {uint16_t protocol,address,command;};
struct Config {uint32_t magic;Binding keys[KEY_COUNT];};
Config saved={},draft={};
Preferences prefs;
BleKeyboard keyboard("PPT Remote","DIY Presenter",100);
NumberInput number;
bool configured=false, learning=false, storageOK=false, wasConnected=false;
int learnIndex=0;
uint32_t learnSince=0,lastFrameAt=0;
Binding lastFrame={};
bool haveFrame=false;
bool equalKey(const Binding& a,const Binding& b){
 return a.protocol==b.protocol && a.address==b.address && a.command==b.command;
}
void prompt(){Serial.printf("Press and RELEASE remote button: %s\n",labels[learnIndex]);}
void startLearning(){
 learning=true;learnIndex=0;draft={};number.clear();learnSince=millis();
 keyboard.releaseAll();
 Serial.println("Learning 14 buttons. X cancels. Old settings stay until completion.");prompt();
}
void tap(uint8_t key){
 if(!keyboard.isConnected())return;
 keyboard.press(key);delay(25);keyboard.releaseAll();delay(25);
}
void sendNumber(){
 unsigned slide=number.value; number.clear();
 if(!keyboard.isConnected() || !slide)return;
 char text[6];snprintf(text,sizeof(text),"%u",slide);
 for(const char* p=text;*p;p++)tap(uint8_t(*p));
 tap(KEY_RETURN);Serial.printf("Slide %u\n",slide);
}
void setup(){
 Serial.begin(115200);delay(300);
 Serial.println("PPT Remote v1 - BLE keyboard / GPIO27 IR");
 storageOK=prefs.begin("ppt-remote",false);
 if(storageOK && prefs.getBytesLength("mapping")==sizeof(saved)){
  prefs.getBytes("mapping",&saved,sizeof(saved));
  configured=saved.magic==0x50505431;
 }
 keyboard.begin();
 IrReceiver.begin(IR_PIN,DISABLE_LED_FEEDBACK);
 Serial.println("Serial: L = learn all buttons; X = cancel learning.");
 if(!storageOK)Serial.println("ERROR: NVS unavailable; settings cannot be saved.");
 if(!configured)Serial.println("No remote configured. Send L to start learning.");
 else Serial.println("Remote settings loaded. Pair PPT Remote in Windows Bluetooth.");
}
void loop(){
 uint32_t now=millis();
 bool connected=keyboard.isConnected();
 if(connected!=wasConnected){
  number.clear();keyboard.releaseAll();wasConnected=connected;
  Serial.println(connected?"Bluetooth CONNECTED":"Bluetooth DISCONNECTED");
 }
 while(Serial.available()){
  char c=Serial.read();
  if(c=='L'||c=='l')startLearning();
  if(c=='X'||c=='x'){learning=false;number.clear();Serial.println("Learning cancelled.");}
 }
 if(learning && uint32_t(now-learnSince)>60000){
  learning=false;Serial.println("Learning timeout. Send L to retry.");
 }
 if(!learning && connected && number.ready(now))sendNumber();
 // Clear a zero-only entry too, so it never lingers indefinitely.
 if(number.count && uint32_t(now-number.last)>=1000)number.clear();
 if(IrReceiver.decode()){
  auto data=IrReceiver.decodedIRData;
  IrReceiver.resume();
  if(data.protocol==UNKNOWN || (data.flags & IRDATA_FLAGS_WAS_OVERFLOW))return;
  Binding key={uint16_t(data.protocol),data.address,data.command};
  bool repeat=(data.flags & IRDATA_FLAGS_IS_REPEAT) ||
    (haveFrame && equalKey(key,lastFrame) && uint32_t(now-lastFrameAt)<250);
  lastFrame=key;lastFrameAt=now;haveFrame=true;
  if(repeat)return;
  Serial.printf("IR protocol=%u address=0x%X command=0x%X\n",key.protocol,key.address,key.command);
  if(learning){
   for(int i=0;i<learnIndex;i++)if(equalKey(key,draft.keys[i])){
    Serial.println("Already assigned. Choose a different button.");return;
   }
   draft.keys[learnIndex++]=key;learnSince=now;
   if(learnIndex<KEY_COUNT){prompt();return;}
   draft.magic=0x50505431;
   if(storageOK && prefs.putBytes("mapping",&draft,sizeof(draft))==sizeof(draft)){
    saved=draft;configured=true;Serial.println("SAVED. Learning finished.");
   }else Serial.println("ERROR saving settings. Send L to retry.");
   learning=false;return;
  }
  if(!configured || !connected)return;
  int action=-1;
  for(int i=0;i<KEY_COUNT;i++)if(equalKey(key,saved.keys[i])){action=i;break;}
  if(action<0)return;
  if(action>=4){number.digit(action-4,now);return;}
  number.clear(); // Navigation/start/exit cancels an unfinished slide number.
  switch(action){
   case 0:tap(KEY_RIGHT_ARROW);break;
   case 1:tap(KEY_LEFT_ARROW);break;
   case 2:tap(KEY_F5);break;
   case 3:tap(KEY_ESC);break;
  }
 }
 delay(1);
}
