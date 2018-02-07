#include <CBC.h>
#include <AES.h>
#include <Servo.h>
#include <BLEPeripheral.h>
#define SIGNAL A0

//An Array to hold all of the pins we care about
int pins[8] = {0,1,2,3,4,5,6,7};

//BLE peripheral object
BLEPeripheral blePeripheral = BLEPeripheral();  

 //size of the data being recieved
int DATA_TRANSFER_SIZE = 16;

//what is being seen as a service
BLEService outletService("19B10000-E8F2-537E-4F6C-D104768A1214");

//IO channels
BLEFixedLengthCharacteristic reader("19B10000-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify, DATA_TRANSFER_SIZE);
BLEFixedLengthCharacteristic writer("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLERead | BLENotify, DATA_TRANSFER_SIZE);




uint8_t stack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
size_t stack_size = 0;
uint8_t iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

size_t len = 16;
uint8_t key[16] = {0x75, 0x71, 0xac, 0x7a, 0x6b, 0xe3, 0x2f, 0xc8, 0x17, 0xb4, 0xb8, 0x17, 0x63, 0xa7, 0xee, 0xfe};


void setup() {
  Serial.begin(9600);
  blePeripheral.setLocalName("HomeModesModule2"); // Set a local name for the BLE device

  blePeripheral.setAdvertisedServiceUuid(outletService.uuid());

  // add service and characteristic:
  blePeripheral.addAttribute(outletService);
  blePeripheral.addAttribute(reader);
  blePeripheral.addAttribute(writer);
 
  // set the initial value for the characeristic:
   char readerInitial[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   char writerInitial[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  reader.setValue(readerInitial);
  writer.setValue(0); 
 
  
  blePeripheral.begin();  //activate BLE device to continuosly transmit BLE advertising packets.
  //Your board will be visible to central devices until it receives a new connection
 

}

//Processes everythign we care about
//Reads EdisonLang and implements it

void processReader(uint8_t* input){
  digitalWrite(BLE_LED, LOW);
   uint8_t output[DATA_TRANSFER_SIZE];
   int outSize = decrypt(input, output);
   int messageID = output[0]*256*256 + output[1]*256 + output[2]; 
   int messageLength = 256 * output[4] + output[5];
   unsigned long startTime;
   byte bitCompareTo;
   
   byte var = 0;
   for (int j,i = 6; i < outSize; i++) {
    uint8_t code = output[i];
    int waitCount;
    switch (code) {
      case 0:
      return;
      case 1:
      //ignore for now
      break;
      case 2:
        stack[stack_size++] = output[++i];
      break;
      case 3:
        stack[stack_size++] = output[++i];
        stack[stack_size++] = output[++i];
      break;
      case 5:
        var = stack[--stack_size];
        stack[stack_size] = 0;
        bitCompareTo = 1;
        for(j= 0; j < sizeof(pins)/sizeof(int); j++){
          if(var & bitCompareTo != 0){
            digitalWrite(pins[i],HIGH);
          }
          else{
            digitalWrite(pins[i],LOW);
          }
          bitCompareTo *= 2;
        }
       
      break;
      case 11:
        waitCount = stack[--stack_size]/15;
        delay(waitCount);
        stack[stack_size] = 0;
      break;
      case 12:
       waitCount = (stack[stack_size-2] * 256 + stack[stack_size -1])/15;
       delay(waitCount);
       stack[stack_size-1] = 0;
       stack[stack_size-2] = 0;
       stack_size -= 2;
      break;
    }
   }
   


   
}
//Loop for stuffs
//Does everyhitng in the program

void loop() {

  BLECentral central = blePeripheral.central();   // listen for BLE peripherals to connect

  if (central) {                             //when connected to a central
     
    while(central.connected()){
      if(reader.written()){
        if(reader.value()){
          digitalWrite(BLE_LED, HIGH);
          uint8_t *dta = (uint8_t*)reader.value();
          uint8_t copy[DATA_TRANSFER_SIZE];

          for (int i = 0; i < DATA_TRANSFER_SIZE; i++) {
            copy[i] = dta[i];
          }
          
          processReader(copy);
          
        }
      }

      delay(15);
    }
    blePeripheral.begin();
 
  }
  
  delay(200);
  digitalWrite(BLE_LED, HIGH);
  delay(200);
  digitalWrite(BLE_LED, LOW);
  delay(600);
  
}


//Yay encryption. 
//Takes in an input, and the numByes to encrypt
//Encrypts them
uint8_t* encrypt(uint8_t* input, int numBytes){
    
    uint8_t val[DATA_TRANSFER_SIZE];

    for (int i = 0; i < numBytes; i++) {
      val[i] = input[i];
    }
    for (int i = numBytes; i < DATA_TRANSFER_SIZE; i++) {
      val[i] = DATA_TRANSFER_SIZE - numBytes;
    }
    
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



// A rather dumb function to decrypt data
int decrypt(uint8_t* input, uint8_t* store){
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
    cbc.decrypt(output,(const uint8_t*)other, DATA_TRANSFER_SIZE);

    int padding = output[DATA_TRANSFER_SIZE-1];
    for (int i = 0; i < DATA_TRANSFER_SIZE - padding; i++) {
      store[i] = output[i];
    }

    return DATA_TRANSFER_SIZE - padding;
}






