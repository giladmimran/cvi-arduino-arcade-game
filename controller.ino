int led_pin = 2;   // LED pin
int fire_pin = 7;  // Fire button pin
int right_pin = 5; // Right movement pin
int left_pin = 4;  // Left movement pin
unsigned long previousMillis = 0;
const long interval = 100;  // Send data every 100 ms

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(fire_pin, INPUT);
  pinMode(right_pin, INPUT);
  pinMode(left_pin, INPUT);
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
}

void loop() {
  unsigned long currentMillis = millis();

  // Send data at a fixed interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read button data
    int fire = digitalRead(fire_pin);
    int right = digitalRead(right_pin);
    int left = digitalRead(left_pin);

    // Send movement data
    if (right == HIGH) {
      Serial.print("1");  // Right movement
    } else if (left == HIGH) {
      Serial.print("9");  // Left movement
    } else {
      Serial.print("0");  // No movement
    }

    // Send firing data
    if (fire == HIGH) {
      Serial.println(" y");  // Fire button pressed
      digitalWrite(led_pin, HIGH);  // LED on when firing
    } else {
      Serial.println(" n");  // Fire button not pressed
      digitalWrite(led_pin, LOW);  // LED off when not firing
    }
  }
}
