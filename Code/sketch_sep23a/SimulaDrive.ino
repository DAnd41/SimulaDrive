#include <BleGamepad.h>
#include <ESP32Encoder.h>

// ================= KONFIGURASI NAMA BLUETOOTH =================
BleGamepad bleGamepad("SimulaDrive Wheel", "Grand House Studio", 100);

// ================= OBJEK ENCODER ==============================
ESP32Encoder steeringEncoder;

// ================= DEFINISI PIN ===============================
const int PIN_ENCODER_A = 18; 
const int PIN_ENCODER_B = 19; 
const int PIN_PEDAL_GAS = 32; 
const int PIN_PEDAL_REM = 33; 

// ================= VARIABEL KALIBRASI =========================
const int MAX_STEERING_ANGLE = 2000;  
const int MIN_STEERING_ANGLE = -2000; 

float gasSmoothed = 0;
float remSmoothed = 0;
const float SMOOTHING_FACTOR = 0.2; 

void setup() {
  Serial.begin(115200);
  
  // 1. PERBAIKAN ENCODER: Menggunakan puType::up untuk versi library terbaru
  ESP32Encoder::useInternalWeakPullResistors = puType::up;
  steeringEncoder.attachFullQuad(PIN_ENCODER_A, PIN_ENCODER_B);
  steeringEncoder.setCount(0); 

  // 2. Konfigurasi Resolusi ADC (Pedal)
  analogReadResolution(12);

  // 3. PERBAIKAN BLUETOOTH: Menggunakan objek BleGamepadConfiguration
  Serial.println("Sistem SimulaDrive siap. Menunggu koneksi Bluetooth...");
  
  BleGamepadConfiguration bleGamepadConfig;
  bleGamepadConfig.setAutoReport(false); 
  bleGamepad.begin(&bleGamepadConfig); 
}

void loop() {
  if (bleGamepad.isConnected()) {
    
    // --- MEMBACA SETIR ---
    long currentSteeringCount = steeringEncoder.getCount();
    currentSteeringCount = constrain(currentSteeringCount, MIN_STEERING_ANGLE, MAX_STEERING_ANGLE);
    int16_t x_Axis = map(currentSteeringCount, MIN_STEERING_ANGLE, MAX_STEERING_ANGLE, -32767, 32767);

    // --- MEMBACA PEDAL ---
    int rawGas = analogRead(PIN_PEDAL_GAS);
    int rawRem = analogRead(PIN_PEDAL_REM);

    // --- FILTER NOISE PEDAL ---
    gasSmoothed = (rawGas * SMOOTHING_FACTOR) + (gasSmoothed * (1.0 - SMOOTHING_FACTOR));
    remSmoothed = (rawRem * SMOOTHING_FACTOR) + (remSmoothed * (1.0 - SMOOTHING_FACTOR));

    int16_t y_Axis = map((int)gasSmoothed, 0, 4095, -32767, 32767);
    int16_t z_Axis = map((int)remSmoothed, 0, 4095, -32767, 32767);

    // --- KIRIM DATA ---
    bleGamepad.setX(x_Axis); 
    bleGamepad.setY(y_Axis); 
    bleGamepad.setZ(z_Axis); 
    bleGamepad.sendReport(); 
  }

  delay(10); 
}