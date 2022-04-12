// I2C Master Mode
// Sends to Address 0x50
// Waits for character entered from serial monitor
// 1 + enter = Read from device
// 2 + enter = Send message 

#include <Wire.h>

void setup()
{
  Wire.begin(); // join i2c bus
  Wire.setClock(100000);
  Serial.begin(9600);
  int incoming = 0;
}

void sendMessage(String message){
  Wire.beginTransmission(0x50);
  Wire.write(message);
  Wire.endTransmission();
}


void readMessage(){
  Serial.print("Incoming message: ");
  Wire.requestFrom(0x50, 1);
  Serial.println(Wire.read());
}

void loop(){
  int incoming = Serial.read();
  if (incoming == 49){
    readMessage();
  }
  if (incoming == 50){
    sendMessage("Hello iOBC");
  }
}
