const int lm35_pin = A0;	/* LM35 O/P pin */

void setup() {
  Serial.begin(9600);
}

void loop() {
  int temp_adc_val;
  float temp_val;
  temp_adc_val = analogRead(lm35_pin);	/* Read Temperature */
   Serial.print("temp_adc_val = ");
  Serial.println(temp_adc_val);
  temp_val = (temp_adc_val * 4.88);	/* Convert adc value to equivalent voltage */
  Serial.print("temp_val = ");
  Serial.print(temp_val);
  Serial.println("mV");
  temp_val = (temp_val/10);	/* LM35 gives output of 10mv/°C */
  Serial.print("Temperature = ");
  Serial.print(temp_val);
  Serial.print(" Degree Celsius\n");
  delay(1000);
}
