
const bool DEBUG = false;
const int MAX = 6;
const unsigned int REVS_DECAY_INTERVAL = 2000;
const unsigned int PRINT_INTERVAL = 200;
float DECAY_SENTINAL = 20000.0;

unsigned int LAST_STATE = LOW;
unsigned int TOTAL_REVS = 0;
float REVS[MAX];
int REVS_POS = 0;
unsigned long REVS_TIMESTAMP;
unsigned long PRNT_TIMESTAMP;
unsigned long DCAY_TIMESTAMP;
unsigned long STRT_TIMESTAMP;


void setup() {
  Serial.begin(9600);

  pinMode(8, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  PRNT_TIMESTAMP = millis();
  resetData(PRNT_TIMESTAMP);
}

void debug(String message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void resetData(unsigned long timestamp) {
  debug("reset");
  
  REVS_TIMESTAMP = timestamp;
  STRT_TIMESTAMP = timestamp;
  TOTAL_REVS = 0;
  
  for (int i = 0; i < MAX; i++) {
    REVS[i] = DECAY_SENTINAL;
    REVS_POS = 0;
  }
}

void writeData(float rpms, unsigned int duration) {
  Serial.println("{ \"rotationsPerMinute\": " + String(rpms) + ", \"totalRotations\": " + String(TOTAL_REVS) + ", \"duration\": " + String(duration / 1000) + " }");
}

void loop() {
  int sensorVal = digitalRead(8);
  unsigned long now = millis();

  /*
   * Handle state change
   */
  if (LAST_STATE != sensorVal) {
    LAST_STATE = sensorVal;
    
    if (sensorVal == LOW) {
      debug("rotation");
      
      float diff = now - REVS_TIMESTAMP;
      REVS_TIMESTAMP = now;
      DCAY_TIMESTAMP = now;

      TOTAL_REVS += 1;
      REVS_POS += 1;
      if (REVS_POS >= MAX) {
        REVS_POS = 0;
      }

      REVS[REVS_POS] = diff;
    }

    delay(16);
  }
  
  
  /*
   * Are we started?
   */
  bool started = false;
  for (int i = 0; i < MAX; i++) {
    if (REVS[i] != DECAY_SENTINAL)
    {
      started = true;
      break;
    }
  }
  
  /*
   * Handle decay
   */
  if (now - DCAY_TIMESTAMP >= REVS_DECAY_INTERVAL) {
    debug("decay");
    
    DCAY_TIMESTAMP = now;
    if (++REVS_POS >= MAX) {
        REVS_POS = 0;
    }

    REVS[REVS_POS] = DECAY_SENTINAL;
  }

  /*
   * Print data if in-cycle
   */
  if ((now - PRNT_TIMESTAMP) >=  PRINT_INTERVAL)
  {
    PRNT_TIMESTAMP = now;

    if (started == false) {
      resetData(now);
      writeData(0, 0);
      return;
    }

    float rotationTime = 0;
    unsigned int rotationTimeLength = 0;
    
    for (int i = 0; i < MAX; i++) {
      float value = REVS[i];

      debug("arry: " + String(value));

      rotationTime += value;
      rotationTimeLength++; 
    }

    unsigned int duration = now - STRT_TIMESTAMP;
    if (rotationTime >= DECAY_SENTINAL * (0.5 * MAX))
    {
      writeData(0, duration);
      return;
    }

    float avgRotationTime = rotationTime / rotationTimeLength;
    float rpm = 60000 / avgRotationTime;
    
    debug("avg: " + String(avgRotationTime));
    debug("rpm: " + String(rpm));
    
    writeData(rpm, duration);
  }
}
