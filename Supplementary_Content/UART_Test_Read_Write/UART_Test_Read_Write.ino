char returned;

void setup() {
  Serial1.begin(115200); // initialize UART
  Serial.begin(115200);  // Initialize serial output for debug
}

void UART_Write() {
  Serial1.print("Hello iOBC");
}

void loop(){
  if (Serial.available()) {
    Serial.read();
    UART_Write();
  }
  if(Serial1.available()>1){
    returned = Serial1.read();
    Serial.println(returned);
  }
}