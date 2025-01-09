int pulsePin = A0;                 // Pulse Sensor purple wire connected to analog pin A0
int blinkPin = 13;                 // Pin to blink LED at each beat

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // Holds raw BPM value
volatile int Signal;                // Holds the incoming raw data
volatile int IBI = 600;             // Time interval between beats (ms)
volatile boolean Pulse = false;     // True when a live heartbeat is detected
volatile boolean QS = false;        // Becomes true when Arduino finds a beat

volatile int rate[10];                      // Array to hold last ten IBI values
volatile unsigned long sampleCounter = 0;   // Used to determine pulse timing
volatile unsigned long lastBeatTime = 0;    // Used to find IBI
volatile int P = 512;                       // Peak in pulse wave, seeded
volatile int T = 512;                       // Trough in pulse wave, seeded
volatile int thresh = 525;                  // Threshold to detect a heartbeat, seeded
volatile int amp = 100;                     // Amplitude of the pulse waveform, seeded
volatile boolean firstBeat = true;          // Used to seed rate array
volatile boolean secondBeat = false;        // Used to seed rate array

void setup() {
  Serial.begin(115200);              // Initialize Serial communication
  Serial.println("Monitorando frequência cardíaca...");
  interruptSetup();                  // Set up the interrupt for the pulse sensor
}

void loop() {
  if (QS == true) {                  // A heartbeat was found
    Serial.print("BPM: ");
    Serial.println(BPM);             // Output the BPM to the Serial Monitor
    QS = false;                      // Reset the Quantified Self flag
  }
  delay(20);                         // Small delay
}

void interruptSetup() {     
  // Initializes Timer2 to throw an interrupt every 2ms
  TCCR2A = 0x02;     // Disable PWM on digital pins 3 and 11, and go into CTC mode
  TCCR2B = 0x06;     // Don't force compare, 256 prescaler 
  OCR2A = 0x7C;      // Set the top of the count to 124 for 500Hz sample rate
  TIMSK2 = 0x02;     // Enable interrupt on match between Timer2 and OCR2A
  sei();             // Enable global interrupts      
} 

ISR(TIMER2_COMPA_vect) {  
  cli();                                      // Disable interrupts while we do this
  Signal = analogRead(pulsePin);              // Read the Pulse Sensor 
  sampleCounter += 2;                         // Increment the sample counter by 2ms
  int N = sampleCounter - lastBeatTime;       // Time since the last beat

  // Find the peak and trough of the pulse wave
  if (Signal < thresh && N > (IBI / 5) * 3) { // Avoid dichrotic noise
    if (Signal < T) {                        
      T = Signal;                             // Track the lowest point in the pulse wave 
    }
  }

  if (Signal > thresh && Signal > P) {        // Threshold condition helps avoid noise
    P = Signal;                               // Track the highest point in the pulse wave
  }

  // Detect a heartbeat
  if (N > 250) {                              // Avoid high frequency noise
    if ((Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3)) {
      Pulse = true;                           // Set the Pulse flag
      IBI = sampleCounter - lastBeatTime;     // Measure time between beats in ms
      lastBeatTime = sampleCounter;           // Update the last beat time
      
      if (secondBeat) {                       // If this is the second beat
        secondBeat = false;                   // Clear the secondBeat flag
        for (int i = 0; i <= 9; i++) {        // Seed the rate array
          rate[i] = IBI;
        }
      }

      if (firstBeat) {                        // If it's the first beat
        firstBeat = false;                    // Clear the firstBeat flag
        secondBeat = true;                    // Set the secondBeat flag
        sei();                                // Enable interrupts
        return;                               // Discard the first IBI value
      }

      // Keep a running total of the last 10 IBI values
      word runningTotal = 0;
      for (int i = 0; i <= 8; i++) {
        rate[i] = rate[i + 1];                // Shift data in the rate array
        runningTotal += rate[i];              // Add up the 9 oldest IBI values
      }

      rate[9] = IBI;                          // Add the latest IBI to the rate array
      runningTotal += rate[9];                // Add the latest IBI to the running total
      runningTotal /= 10;                     // Average the last 10 IBI values
      BPM = 60000 / runningTotal;             // Calculate BPM
      QS = true;                              // Set Quantified Self flag 
    }                       
  }

  if (Signal < thresh && Pulse == true) {     // When the values are going down
    Pulse = false;                            // Reset the Pulse flag
    amp = P - T;                              // Get the amplitude of the pulse wave
    thresh = amp / 2 + T;                     // Set the threshold to 50% of amplitude
    P = thresh;                               // Reset peak
    T = thresh;                               // Reset trough
  }

  if (N > 2500) {                             // If 2.5 seconds go by without a beat
    thresh = 512;                             // Reset threshold
    P = 512;                                  // Reset peak
    T = 512;                                  // Reset trough
    lastBeatTime = sampleCounter;             // Update last beat time        
    firstBeat = true;                         // Set these to avoid noise
    secondBeat = false;                       // When the heartbeat returns
  }

  sei();                                      // Enable interrupts
}
