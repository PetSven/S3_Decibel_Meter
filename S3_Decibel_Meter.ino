// Created by Reddit user PeterRevision
// 3/3/26

// Imports
#include <M5Unified.h>
#include <arduinoFFT.h>

// Constants
static constexpr int SAMPLE_RATE = 44000; // The sample rate for recording in Hz
static constexpr size_t SAMPLES = SAMPLE_RATE/10; // The length of the array
static int16_t DATA[SAMPLES]; // Pointer to store the raw audio data

// put your setup code here, to run once:
void setup() {

  // Disable the devices I am not using
  auto cfg = M5.config(); 
  cfg.output_power = false;
  cfg.internal_imu = false;
  cfg.internal_spk = false;
  cfg.external_imu = false;
  cfg.external_rtc = false;

  // Start the S3
  M5.begin(cfg);

  // DISABLE EXTERAL 5V IF MODULE MAKES NOISE
  //  M5.Power.setExtOutput(false);

  // The microphone and speaker cannot be used at the same time
  //  M5.Speaker.end();

  // Configure the display
  M5.Display.setTextSize(3);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(128);

  // Enable the microphone
  M5.Mic.begin();

  // Open serial port at 115200 baud
//  Serial.begin(115200);   
//  while (!Serial) { } // Optional: wait for USB serial to be ready
}

void loop() {

  // Record ten audio slices in a second 
  float maxVolume = 0;
  for (int i = 0; i < 10; i++){

    // Record the audio
    M5.Mic.record(DATA, SAMPLES, SAMPLE_RATE, false);

//    while(M5.Mic.isRecording()){
//      delay(1);
//    }

    // Calculate the RMS
    double sum = 0;
//    Serial.print("Start\n");
    for (int i = 0; i < SAMPLES; i++) {
//        Serial.println(DATA[i]);
        float s = DATA[i] / 32768.0f; // Normalize 16 bit integer to +/- 1
        sum += s * s; // Square
    }
    float mean = sum / SAMPLES; // Mean
    float rms = sqrt(mean); // Root
//    Serial.print("Stop\n");
    
    // Calculate the full-scale decibel value
    float dbFS = 20 * log10(rms + 1e-12);
  
    //Fix offset
    float dbSPL = dbFS + 96;

    // Only save the loudest of the 10 samples
    if(dbSPL > maxVolume){
      maxVolume = dbSPL;
    }
  }
      
  // Reset the draw position and clear the screen
  M5.Display.setCursor(0, 20);
  M5.Display.clear();
  
  // Output the result in red 
  if(maxVolume < 85){
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.printf(" Volume:\n %2.1f dB SPL\n Safe", maxVolume);
  }
  else{
    M5.Display.setTextColor(RED, BLACK);
    // "Limit Exposure:" is 3 characters too long
    M5.Display.printf(" Volume:\n %2.1f dB SPL\n Limit Time:\n", maxVolume);

    // Calculate safe exposure time
    float dbOver = maxVolume - 85;
    float safeTime = 8 / pow(2, dbOver/3);
    if(safeTime < 1){
       safeTime = safeTime * 60;
       if(safeTime < 1){
          safeTime = safeTime * 60;
          M5.Display.printf(" %1.0f Seconds", safeTime);
       }
       else{
         M5.Display.printf(" %1.0f Minutes", safeTime);
       }
    }
    else{
      M5.Display.printf(" %1.1f Hours", safeTime);
    }
  }
}
