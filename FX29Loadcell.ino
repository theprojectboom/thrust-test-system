// https://www.mouser.com/datasheet/2/418/5/ENG_DS_FX29_A5-1609196.pdf

#include <Wire.h>

constexpr int LOG_INTERVAL = 500;

constexpr int I2C_ADDRESS = 0x28;
constexpr int SCALE_SPAN  = 1500;
constexpr int ZERO_OFFSET = 1000;

constexpr double LBF_CONV = 100.0       / double(SCALE_SPAN - ZERO_OFFSET);
constexpr double N_CONV   = 444.8221615 / double(SCALE_SPAN - ZERO_OFFSET);

int calib_offset = 0;
bool sending = false;
unsigned long start_time = 0;

int read_raw() {
   // Read_MR command
   Wire.requestFrom(I2C_ADDRESS, 0);

   // Read_DF2 command
   if(Wire.requestFrom(I2C_ADDRESS, 2)) {
      int val = 0;
      val |= (Wire.read() & 0x3f) << 8;
      val |= Wire.read() & 0xff;
      return val;
   }

   return 0;
}

void setup() {
   Serial.begin(9600);
   Wire.begin();
}

void loop() {
   static unsigned long last_time = 0;
   unsigned long now = millis();

   int data_raw = read_raw();
   int data_offset = data_raw - calib_offset - ZERO_OFFSET;

   double data_lbf = data_offset * LBF_CONV;
   double data_n = data_offset * N_CONV;

   if(Serial.available()) {
      switch(Serial.read()) {
         case 'b':
            start_time = now;
            last_time = 0;
            sending = true;
            break;
         case 'e':
            sending = false;
            break;
         case 'z':
            calib_offset = data_raw - ZERO_OFFSET;
            break;
      }
   }

   if(sending && now - last_time > LOG_INTERVAL) {
      Serial.print(now);
      Serial.print(',');
      Serial.print(data_raw);
      Serial.print(',');
      Serial.print(calib_offset);
      Serial.print(',');
      Serial.print(data_lbf);
      Serial.print(',');
      Serial.print(data_n);
      Serial.println();
      last_time = now;
   }
}
