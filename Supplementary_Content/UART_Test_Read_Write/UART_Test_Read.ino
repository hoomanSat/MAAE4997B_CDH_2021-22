char returned;

void setup() {
  //Serial1.begin(115200); // initialize UART
  Serial.begin(115200);  // Initialize serial output for debug
}

void loop(){
  Serial.print("123a");
  Serial.read();
}
