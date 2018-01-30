#include <BLEPeripheral.h>


#define SIGNAL A0


BLEPeripheral blePeripheral;  
   
BLEService servoService(" 19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic reader("19B10000-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify,1);
BLECharacteristic spinNum("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify,1);

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
  Serial.println("Bluetooth device active, waiting for connections...");

}


void loop() {

  BLECentral central = blePeripheral.central();   // listen for BLE peripherals to connect

  if (central) {                             //when connected to a central
    Serial.print("Connected to central: ");
    Serial.println(central.address());  
    while(central.connected()){
      if(reader.written()){
        if(reader.value()){
          spinNum.setValue(reader.value() + 1) //write the value + 1 to the other one
        }
      }
      
    }
 
  }
}
