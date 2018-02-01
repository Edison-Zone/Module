#include <CBC.h>
#include <AES.h>

#include <BLEPeripheral.h>
#define SIGNAL A0


BLEPeripheral blePeripheral = BLEPeripheral();  
 
int DATA_TRANSFER_SIZE = 32;
  
BLEService servoService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEFixedLengthCharacteristic reader("19B10000-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify, DATA_TRANSFER_SIZE);
BLEFixedLengthCharacteristic spinNum("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify, DATA_TRANSFER_SIZE);

uint8_t iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

size_t len = 16;
uint8_t key[16] = {0x75, 0x71, 0xac, 0x7a, 0x6b, 0xe3, 0x2f, 0xc8, 0x17, 0xb4, 0xb8, 0x17, 0x63, 0xa7, 0xee, 0xfe};

void setup() {
  Serial.begin(9600);
  Serial.println("START PROG");

  char string[] = "0118999881999119725 3";
  uint8_t* original = (uint8_t*)string;
  int numBytes = strlen(string);

  blePeripheral.setLocalName("HomeModesModule1"); // Set a local name for the BLE device

  blePeripheral.setAdvertisedServiceUuid(servoService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(servoService);
  blePeripheral.addAttribute(reader);
  blePeripheral.addAttribute(spinNum);

  // set the initial value for the characeristic:
  spinNum.setValue(0);
  reader.setValue(0);

  uint8_t* output = encrypt(original, numBytes);
  uint8_t  thing[DATA_TRANSFER_SIZE];
  for(int i = 0; i < DATA_TRANSFER_SIZE; i++){
    thing[i] = output[i];
  }
  uint8_t *output2 = decrypt(thing);
  
  Serial.print("[");
  for (int i = 0; i < numBytes; i++) {
    if (i != 0) {
      Serial.print(", ");
    }
    Serial.print((char)original[i]);
  }
  Serial.println("]");
  Serial.print("[");
  for (int i = 0; output2[i] != 0; i++) {
    if (i != 0) {
      Serial.print(", ");
    }
    Serial.print((char)output2[i]);
  }
  Serial.println("]");
 
  
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
          char secondChars[16]; 
          for (int i = 0; i < 16; i++){
              secondChars[i] = (char)(chars[i] + 1);
          }
          
          spinNum.setValue(secondChars); //write the value + 1 to the other one
        }
      }
      
    }
    blePeripheral.begin();
 
  }
  
  delay(200);
  digitalWrite(BLE_LED, HIGH);
  delay(200);
  digitalWrite(BLE_LED, LOW);
  delay(600);
  
}

uint8_t* encrypt(uint8_t* input, int numBytes){
    
    uint8_t val[DATA_TRANSFER_SIZE];

    for (int i = 0; i < numBytes; i++) {
      val[i] = input[i];
    }
    for (int i = numBytes; i < DATA_TRANSFER_SIZE; i++) {
      val[i] = DATA_TRANSFER_SIZE - numBytes;
    }
    Serial.println(sizeof(input)/sizeof(uint8_t));
  Serial.print("[");
  for (int i = 0; i < sizeof(val)/sizeof(uint8_t); i++) {
    if (i != 0) {
      Serial.print(", ");
    }
    Serial.print(val[i]);
  }
  Serial.println("]");
    
    uint8_t output[DATA_TRANSFER_SIZE];
    for(int i = 0; i < DATA_TRANSFER_SIZE; i++){
      output[i] = 0;
    }
    
    CBC<AES128> cbc;
    cbc.setKey((const uint8_t*)key,len);
    cbc.setIV((const uint8_t*)iv,len);
    cbc.encrypt((uint8_t*)output, (const uint8_t*)val, DATA_TRANSFER_SIZE);
    return output;
}

uint8_t* decrypt(uint8_t* input){
    uint8_t other[DATA_TRANSFER_SIZE];
    for(int i = 0; i < DATA_TRANSFER_SIZE; i++){
       other[i] = input[i];
    }
   
    uint8_t output[DATA_TRANSFER_SIZE];
    for (int i = 0; i < DATA_TRANSFER_SIZE; i++) {
      output[i] = 0;
    }

    CBC<AES128> cbc;
    cbc.setKey((const uint8_t*)key,len);
    cbc.setIV((const uint8_t*)iv,len);
    cbc.decrypt(output,(const uint8_t*)other, sizeof(other)/sizeof(uint8_t));

    int padding = output[DATA_TRANSFER_SIZE-1];
    uint8_t depadded[DATA_TRANSFER_SIZE - padding + 1];
    for (int i = 0; i < DATA_TRANSFER_SIZE - padding; i++) {
      depadded[i] = output[i];
    }
    depadded[DATA_TRANSFER_SIZE - padding] = 0;

    return depadded;
}






