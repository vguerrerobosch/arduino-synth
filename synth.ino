// Frequency values in Hz for each note with compensation
// for Arduino propagation delays. No rocket science here,
// tweeked in by ear using a digital piano as reference.
const int NOTE_C2 = 79;
const int NOTE_Db2 = 83;
const int NOTE_D2 = 88;
const int NOTE_Eb2 = 93;
const int NOTE_E2 = 98;
const int NOTE_F2 = 103;
const int NOTE_Gb2 = 109;
const int NOTE_G2 = 115;
const int NOTE_Ab2 = 121;
const int NOTE_A2 = 128;
const int NOTE_Bb2 = 135;
const int NOTE_B2 = 142;
const int NOTE_C3 = 149;
const int NOTE_Db3 = 157;
const int NOTE_D3 = 165;
const int NOTE_Eb3 = 173;
const int NOTE_E3 = 182;
const int NOTE_F3 = 191;

// Pin numbers for the 18 keys with internal pull-ups enabled
const int KEYS[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, A1, A2, A3, A4, A5};

// Notes for the 18 keys
const int NOTES[] = {NOTE_C2, NOTE_Db2, NOTE_D2, NOTE_Eb2, NOTE_E2, NOTE_F2, NOTE_Gb2, NOTE_G2, NOTE_Ab2, NOTE_A2, NOTE_Bb2, NOTE_B2, NOTE_C3, NOTE_Db3, NOTE_D3, NOTE_Eb3, NOTE_E3, NOTE_F3};

// Pin numbers for audio output and trigger output
const int AUDIO_OUTPUT_PIN = 13;
const int TRIGGER_OUTPUT_PIN = A0;

volatile long t;
float period;
volatile float pulseWidth = 0;
volatile float pulseWidthMax = 0;
volatile float pulseWidthMin = 0;
volatile byte direction = 0;
volatile float rate = 0;
volatile float pulseK = 0;
float freq = 0.0;
float lastFreq = 0.0;


void setup() {
  // Initialize input pins for the 18 keys
  for (int i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); i++) {
    pinMode(KEYS[i], INPUT_PULLUP);
  }

  // Initialize output pins for audio and trigger
  pinMode(AUDIO_OUTPUT_PIN, OUTPUT);
  pinMode(TRIGGER_OUTPUT_PIN, OUTPUT);

  // Wait for a moment to allow the pins to stabilize
  delay(50);

  // Disable interrupts
  cli();

  // Set up Timer 1 for audio output

  // Clear control register for Timer 1
  TCCR1B = 0;

  // Turn on Clear Timer on Compare Match (CTC) mode
  // The "WGM12" bit is the Waveform Generation Mode (WGM) bit for Timer/Counter 1
  TCCR1B |= (1 << WGM12);

  // Use the system clock and a prescaler of 1
  // The "CS10" bit is the Clock Select (CS) bit for Timer/Counter 1
  TCCR1B |= (1 << CS10);

  // Enable output compare interrupt for Timer 1.
  // The "OCIE1A" bit stands for "Output Compare Interrupt Enable 1A"
  TIMSK1 |= (1 << OCIE1A);

  // Enable interrupts
  sei();
}

// Interrupt Service Routines for Audio Output with PWM
ISR(TIMER1_COMPA_vect) {
  // Set the output value of port pin PB5 (digital pin 13) to HIGH/LOW
  if (t < pulseWidth) {
    PORTB |= (1 << PB5);  // Set the fifth bit of PORTB to 1
  } else {
    PORTB &= ~(1 << PB5); // Set the fifth bit of PORTB to 0
  }

  // Check Period and Adjust Pulse Width
  if (t >= period) {
    t = 0;

    // If pulse width is at minimum change direction
    if (pulseWidth <= pulseWidthMin) {
      direction = 1;
    }

    // If pulse width is at maximum change direction
    if (pulseWidth >= pulseWidthMax) {
      direction = 0;
    }

    // Increase/decrease pulseWidth
    pulseWidth += (direction == 0) ? -rate : rate;
  }

  t++;
}


void loop() {
  freq = 0;

  // Check if any keys are pressed and
  // set frequency based on which key is pressed
  for (int i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); i++) {
    if (digitalRead(KEYS[i]) == LOW) { freq = NOTES[i]; }
  }

  // Trigger output pin when the frequency changes
  if (lastFreq != freq) {
    digitalWrite(TRIGGER_OUTPUT_PIN, HIGH);
    digitalWrite(TRIGGER_OUTPUT_PIN, LOW);

    lastFreq = freq;
  }

  if (freq == 0) {
    period = 0;
    pulseWidth = 0;
  } else {
    // convert frequency value to period
    period = 50000 / freq;
  }

  pulseWidthMax = period * 0.5;
  pulseWidthMin = period * 0.1;
  pulseK = pulseWidthMax - pulseWidthMin; // period * 0.4

  // sets PWM speed, 40000 sounds nice
  rate = pulseK * period / 40000; // 16000 / (freq * freq);
}
