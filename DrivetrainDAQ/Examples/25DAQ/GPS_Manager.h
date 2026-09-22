#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// GPS sensor ID
const uint8_t GPS_ID = 0;

// A tidy little struct for GPS data
struct GPS_Data {
  uint32_t time;
  float latitude;
  float longitude;
  float altitude;
  float speed;
};

// All the GPS related code is organized here
class GPS_Manager {

  // Private members
  private:

    // Hardware configuration
    const int GPS_BAUD = 9600;
    const int RX_PIN = 21;
    const int TX_PIN = 22;
    TinyGPSPlus gps;
    HardwareSerial gpsSerial;

  // Public members
  public:

    // Constructor - assign gpsSerial to UART hardware port 1
    GPS_Manager() : gpsSerial(1) {}

    // Initialize the GPS
    void initialize() {
      gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
      Serial.println("GPS initialized.");
    }

    // Process the incoming GPS data
    void empty_buffer() {
      while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
      }
    }

    // Return the current GPS readings
    GPS_Data fetch_data() {
      GPS_Data data;
      data.time      = gps.time.isValid()     ? gps.time.value()      : 0;
      data.latitude  = gps.location.isValid() ? gps.location.lat()    : 0.0;
      data.longitude = gps.location.isValid() ? gps.location.lng()    : 0.0;
      data.altitude  = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
      data.speed     = gps.speed.isValid()    ? gps.speed.kmph()      : 0.0;
      return data;
    }
};

#endif