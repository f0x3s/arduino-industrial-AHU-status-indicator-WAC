
/*
  Industrial Fan State Monitor

  <desc>

  see details: github link

  created 10 Jun 2026
  by foxes
  modified 11 Jun 2026
  by foxes


*/

#include <math.h>

const int trigPin = 5;
const int echoPin = 6;

const unsigned long REST_INTERVAL_US = 50000;
const unsigned long CLEAR_TIME_US = 2;
const unsigned long TRIG_TIME_US = 10;
const unsigned long ECHO_TIMEOUT_US = 30000;

const int maxDist = 50;

enum SensorState {
  RESTING,
  CLEAR_TRIGGER,
  SEND_TRIGGER,
  WAIT_FOR_ECHO_START,
  WAIT_FOR_ECHO_END
};

SensorState state = RESTING;

unsigned long stateStartMicros = 0;
unsigned long echoStartMicros = 0;
unsigned long lastReadingMicros = 0;

int distance = 0;

float thresh = 30;

const size_t BUFFER_SIZE = 50;
float buffer[BUFFER_SIZE];
size_t head = 0;
size_t count = 0;

bool newReading = false;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);

  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    buffer[i] = 0;
  }
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  
}

void loop() {
  unsigned long now = micros();

  newReading = false;

  checkSensor(now);
  
  if (newReading && count > 1 && fabs(getMax() - getMin()) > thresh) {
    digitalWrite(LED_BUILTIN, HIGH);

  } else if (newReading) {
    digitalWrite(LED_BUILTIN, LOW);

  }
}

void checkSensor(unsigned long now) {
  switch (state) {
    
    case RESTING:
      if (now - lastReadingMicros >= REST_INTERVAL_US) {
        digitalWrite(trigPin, LOW);
        stateStartMicros = now;
        state = CLEAR_TRIGGER;
      }
      break;

    case CLEAR_TRIGGER:
      if (now - stateStartMicros >= CLEAR_TIME_US) {
        digitalWrite(trigPin, HIGH);
        stateStartMicros = now;
        state = SEND_TRIGGER;
      }
      break;

    case SEND_TRIGGER:
      if (now - stateStartMicros >= TRIG_TIME_US) {
        digitalWrite(trigPin, LOW);
        stateStartMicros = now;
        state = WAIT_FOR_ECHO_START;
      }
      break;

    case WAIT_FOR_ECHO_START:
      if (digitalRead(echoPin) == HIGH) {
        echoStartMicros = now;
        state = WAIT_FOR_ECHO_END;
      }

      if (now - stateStartMicros >= ECHO_TIMEOUT_US) {
        lastReadingMicros = now;
        state = RESTING;
      }
      break;

    case WAIT_FOR_ECHO_END:
      if (digitalRead(echoPin) == LOW) {
        unsigned long duration = now - echoStartMicros;

        distance = duration * 0.034 / 2;

        if (distance > maxDist) {
          distance = maxDist;
        }

        addToBuffer(distance);
        newReading = true;

        lastReadingMicros = now;
        state = RESTING;
      }

      if (now - echoStartMicros >= ECHO_TIMEOUT_US) {
        lastReadingMicros = now;
        state = RESTING;
      }
      break;
  }
}

void addToBuffer(float value) {
  if (count < BUFFER_SIZE) {
    count++;
  }

  buffer[head] = value;
  head = (head + 1) % BUFFER_SIZE;
}

float getMax() {
  if (count == 0) return NAN;
  
  float maxVal = buffer[0];
  
  for (size_t i = 1; i < count; i++) {
    if (buffer[i] > maxVal) {
      maxVal = buffer[i];
    }
  }
  
  return maxVal;
}

float getMin() {
  if (count == 0) return NAN;
  
  float minVal = buffer[0];
  
  for (size_t i = 1; i < count; i++) {
    if (buffer[i] < minVal) {
      minVal = buffer[i];
    }
  }
  
  return minVal;
}
