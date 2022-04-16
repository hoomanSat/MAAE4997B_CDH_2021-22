// Slave on the I2C Bus at address 0x41
// Receives or sends a message on request

#include <Wire.h>
void setup()
{
  Wire.begin(0x41);
  Wire.setClock(100000);
  
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}
void loop()
{
  delay(500);
}

char message;
void receiveEvent(int howMany)
{
  while(0 < Wire.available())
  {
    message = Wire.read();
    Serial.print(message);
  }
  Serial.println();
}

void requestEvent() {
  Wire.write("Hello iOBC");
}
