void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.setRX(1);
  Serial1.setTX(0);
  Serial1.begin(9600);
}
void loop() {
  if (Serial.available())
  {
    Serial1.write(Serial.read());
  }

  if (Serial1.available())
  {
    Serial.write(Serial1.read());
  }
}