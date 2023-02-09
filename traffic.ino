/*

      CIS-73-01: IoT Fundamentals
      Cabrillo Collete, Aptos, CA

      Marcos R. Della (mdella@gmail.com)

      Homework 01
      Stoplight functionality w/walk lights

*/

#define WALK_NS_LED 9
#define WALK_EW_LED 3

#define WALK_NS_BUTTON 8
#define WALK_EW_BUTTON 2

#define GREEN_NS_LED_DELAY 5000
#define GREEN_EW_LED_DELAY 5000
#define YELLOW_LED_DELAY 1000

#define WALK_WHITE_TIME 2000
#define WALK_RED_TIME 8000

#define BLINK_DELAY 500

// Pin assignments for various LEDs
unsigned int ledPin[] = {
  4,    // (0b00000001) N/A
  7,    // (0b00000010) RED North/South led
  6,    // (0b00000100) YELLOW North/South led
  5,    // (0b00001000) GREEN North/South led
  10,   // (0b00010000) N/A
  13,   // (0b00100000) RED East/West led
  12,   // (0b01000000) YELLOW East/West led
  11,   // (0b10000000) GREEN East/West led
};

// Light pattern to display in various states
unsigned int light_pattern[][3] = {
  { 0b00110011, 0, 250 },                  // NS=red, EW=red
  { 0b00111001, 2, GREEN_EW_LED_DELAY },   // NS=red, EW=green, enable walk EW(2)
  { 0b00110101, 0, YELLOW_LED_DELAY },     // NS=red, EW=yellow
  { 0b10010011, 1, GREEN_NS_LED_DELAY },   // NS=green, EW=red, enable walk NS(1)
  { 0b01010011, 0, YELLOW_LED_DELAY },     // NS=yellow, EW=red
};

// State definition
//    0 = init
//    1 = North/South green, East/West red
//    2 = North/South yellow, East/West red
//    3 = North/South red, East/West green
//    4 = North/South red, East/West yellow
//    5 = North/South blinking red, East/West blinking red
unsigned int state = 0;
unsigned long endDelay = 0;
unsigned long blinkDelay = 0;

bool ns_walk = false;
bool ew_walk = false;
bool ns_press = false;
bool ew_press = false;

void setup() {
  int i = 0;

  Serial.begin(9600);
  while (i < 8) {
    if (ledPin[i] > 0)
      pinMode(ledPin[i], OUTPUT);
    i++;
  };

  pinMode(WALK_NS_LED, OUTPUT);
  pinMode(WALK_EW_LED, OUTPUT);
  pinMode(WALK_NS_BUTTON, INPUT);
  pinMode(WALK_EW_BUTTON, INPUT);

  lightChange(light_pattern[0][0]);         // Initialize light pattern to Red/Red
  //digitalWrite(NOWALK_NS_LED, HIGH);        // Set do-not-walk North/South  
  //digitalWrite(NOWALK_EW_LED, HIGH);        // Set do-not-walk East/West
  lightChange(0b11111111);
  delay(1000);
  lightChange(0b00000000);
  delay(1000);
  lightChange(0b11111111);
  delay(1000);
  lightChange(0b00000000);  
};

void lightChange(unsigned int lightOut) {
  Serial.print("LED: ");
  for (int i=0; i<8; i++) {
    if ((lightOut >> i) & (0x01)) {
      Serial.print(" 1 ");
      digitalWrite(ledPin[i],HIGH);
    } else {
      Serial.print(" 0 ");
      digitalWrite(ledPin[i],LOW);
    }
  }
}

void loop() {
  unsigned long lightTime;
  unsigned long currTime;

  // If our delay timer runs out, change state. Only executed at state change
  currTime = millis();
  if (currTime > endDelay) {
    Serial.println("");
    if (++state > 4)
      state = 1;
    lightTime = light_pattern[state][2];
    switch (state) {
      case 1:
        if (ew_press == true) {
          ew_walk = true;
          if (WALK_WHITE_TIME + WALK_RED_TIME > lightTime)
            lightTime = WALK_WHITE_TIME + WALK_RED_TIME;
        }
        break;

      case 2:
        ew_walk == false;
        break;

      case 3:
        if (ns_press == true) {
          ns_walk = true;
          if (WALK_WHITE_TIME + WALK_RED_TIME > lightTime)
            lightTime = WALK_WHITE_TIME + WALK_RED_TIME;
        }
        break;

      case 4:
        ns_walk == false;
        break;
    }
    endDelay = currTime + lightTime;
    Serial.print("State ["+String(state) + "]  ");
    lightChange(light_pattern[state][0]);
    //digitalWrite(WALK_NS_LED, LOW);
    Serial.print("    Delay: "+String(light_pattern[state][2])+"ms    ");
  }

  if (currTime > blinkDelay) {
    switch (state) {
      case 1:               // EW light is green
        if (ew_walk) {
          if (currTime > endDelay - (lightTime - WALK_WHITE_TIME) ) {
            // Blink red light
            Serial.print("R");
            digitalWrite(ledPin[0],digitalRead(ledPin[0])^1);
            digitalWrite(WALK_EW_LED,LOW);
          } else {
            // Blink white light
            Serial.print("1");
            digitalWrite(ledPin[0],LOW);
            //digitalWrite(WALK_EW_LED, digitalRead(WALK_EW_LED)^1);
            digitalWrite(WALK_EW_LED,HIGH);
            ew_press = false;
          }
        }
        break;

      case 2:               // EW light changes to yellow
        ew_walk = false;
        digitalWrite(ledPin[0],HIGH);
        break;

      case 3:               // NS light is green
        if (ns_walk) {
          if (currTime > endDelay - (lightTime - WALK_WHITE_TIME) ) {
            // Blink red light
            Serial.print("R");
            digitalWrite(ledPin[4],digitalRead(ledPin[4])^1);
            //if (digitalRead(ledPin[4])) {
            //  tone(8,523);
            //} else {
            //  noTone(8);
            //}
            digitalWrite(WALK_NS_LED,LOW);
          } else {
            // Blink while light
            Serial.print("3");
            digitalWrite(ledPin[4],LOW);
            //digitalWrite(WALK_NS_LED, digitalRead(WALK_NS_LED)^1);
            digitalWrite(WALK_NS_LED,HIGH);
            ns_press = false;
          }
        }
        break;
      case 4:               // NS light changes to yellow
        ns_walk = false;
        digitalWrite(ledPin[4],HIGH);
        break;
    }
    blinkDelay = currTime + BLINK_DELAY;
  }
  
  // check buttons
  if ((ns_press == false) && (digitalRead(WALK_NS_BUTTON) == HIGH)) {
    ns_press = true;
    Serial.print("[NS] ");
  }
  if ((ew_press == false) && (digitalRead(WALK_EW_BUTTON) == HIGH)) {
    ew_press = true;
    Serial.print("[EW] ");
  }

}

