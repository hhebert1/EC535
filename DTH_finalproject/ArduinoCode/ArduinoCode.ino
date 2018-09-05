
// Initialize variables to receive data
int state = 0;
int old_state = 0;

// Pinouts for arduino to gumstix connection
int upPin = 31;
int downPin = 33;
int leftPin = 35;
int rightPin = 37;

void setup() {
  
    // Sets the pins as outputs:
    pinMode(upPin, OUTPUT);
    pinMode(downPin, OUTPUT);
    pinMode(leftPin, OUTPUT);
    pinMode(rightPin, OUTPUT);
    
    // Initialize them all to low so no commands sent to gumstix
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, LOW);
    digitalWrite(leftPin, LOW);
    digitalWrite(rightPin, LOW);
  
    // Default connection rate for my BT module
    Serial.begin(9600); 
    //Serial1.begin(9600);
}


void loop() {
  
    // If some data is sent, read it and save it in the state variable
    if (Serial.available() > 0) {
      state = Serial.read();
    }

    // Only send a command if it is different from the old state
    if (old_state != state)
    {
      if (state == 117)
      {
        // "u" character sent by Kinect App
        Serial.println("up gesture");
        digitalWrite(upPin, HIGH);
        digitalWrite(downPin, LOW);
        digitalWrite(leftPin, LOW);
        digitalWrite(rightPin, LOW);
      }
      if (state == 108)
      {
        // "l" character sent by Kinect App
        Serial.println("left gesture");
        digitalWrite(upPin, LOW);
        digitalWrite(downPin, LOW);
        digitalWrite(leftPin, HIGH);
        digitalWrite(rightPin, LOW);
      }
      if (state == 100)
      {
        // "d" character sent by Kinect App
        Serial.println("down gesture");
        digitalWrite(upPin, LOW);
        digitalWrite(downPin, HIGH);
        digitalWrite(leftPin, LOW);
        digitalWrite(rightPin, LOW);
      }
      if (state == 114)
      {
        // "r" character sent by Kinect App
        Serial.println("right gesture");
        digitalWrite(upPin, LOW);
        digitalWrite(downPin, LOW);
        digitalWrite(leftPin, LOW);
        digitalWrite(rightPin, HIGH);
      }
      if (state != 1)
      {
        old_state = state;
      }
    }
}
