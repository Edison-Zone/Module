#include <BLEPeripheral.h>


#define SIGNAL A0


BLEPeripheral blePeripheral = BLEPeripheral();  
   
BLEService servoService(" 19B10000-E8F2-537E-4F6C-D104768A1214");
BLEFixedLengthCharacteristic reader("19B10000-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify,10);
BLEFixedLengthCharacteristic spinNum("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify,10);

void setup() {
  Serial.begin(9600);

  blePeripheral.setLocalName("HomeModesModule1"); // Set a local name for the BLE device

  blePeripheral.setAdvertisedServiceUuid(servoService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(servoService);
  blePeripheral.addAttribute(reader);
  blePeripheral.addAttribute(spinNum);

  // set the initial value for the characeristic:
  spinNum.setValue(0);
  reader.setValue(0);

 

  blePeripheral.begin();  //activate BLE device to continuosly transmit BLE advertising packets.
  //Your board will be visible to central devices until it receives a new connection
 

}


void loop() {

  BLECentral central = blePeripheral.central();   // listen for BLE peripherals to connect

  if (central) {                             //when connected to a central
     
    while(central.connected()){
      if(reader.written()){
        if(reader.value()){
          const unsigned char* chars = reader.value();
          char secondChars[10]; 
          for (int i = 0; i < 10; i++){
              secondChars[i] = (char)(chars[i] + 1);
          }
          
          spinNum.setValue(secondChars); //write the value + 1 to the other one
        }
      }
      
    }
 
  }
}


