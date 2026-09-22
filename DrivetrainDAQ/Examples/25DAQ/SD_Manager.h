#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "GPS_Manager.h"
#include "RPM_Manager.h"
#include <SPI.h>
#include <SD.h>

// All the SD card related code is organized here
class SD_Manager {

  // Private members
  private:

    // Hardware configuration
    const int CS_PIN   = 5;
    const int SCK_PIN  = 18;
    const int MISO_PIN = 19;
    const int MOSI_PIN = 23;

    // Log file information
    uint8_t buffer[4096];
    uint16_t buffer_index = 0;
    String FILENAME = "/log0.bin";
    File log;

    // Write data to the buffer
    template <typename DataType>
    void write_to_buffer(const DataType& data) {
      memcpy(&buffer[buffer_index], &data, sizeof(data));
      buffer_index += sizeof(data);
    }

  // Public members
  public:

    // Initialize the SD card
    void initialize() {

      // Attempt to initialize the card
      SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
      if (!SD.begin(CS_PIN)) {

          // Report failure and halt the program
          Serial.println("ERROR: SD Card Mount Failed!");
          while (1);
      }

      // Report success
      Serial.println("SD Card mounted successfully!");
      
      // Read 
      String log_number = "X";
      File log_number_file = SD.open("/number.txt", FILE_READ);
      if (log_number_file) {
        log_number = log_number_file.readString();
        String next_log_number = String(log_number.toInt() + 1);

        log_number_file.close();
        log_number_file = SD.open("/number.txt", FILE_WRITE);
        if (log_number_file) {
          log_number_file.print(next_log_number);
          log_number_file.close();
        }
      }

      FILENAME = "/log" + log_number + ".bin";
      
      // Create a log file (or empty it if one already exists)
      log = SD.open(FILENAME, FILE_WRITE);
      if (log) {
        log.close();
      }     
      
      // Reopen the log file in append mode
      log = SD.open(FILENAME, FILE_APPEND);
    }
    
    // Log a 5-byte sensor pulse to the buffer (1-byte sensor ID + 4-byte timestamp)
    void log_sensor_pulse(uint8_t sensor_id, uint32_t timestamp) {
      
      // Debugging option - print every sensor pulse to the console
      // Serial.printf("Sensor ID: %u | Timestamp: %lu\n", sensor_id, timestamp);

      // Flush the buffer if it cannot fit 5 more bytes
      if (buffer_index + 5 > sizeof(buffer)) {
        flush_to_log();
      }

      // Write the sensor ID (1 byte) and timestamp (4 bytes) to the buffer
      write_to_buffer(sensor_id);
      write_to_buffer(timestamp);
    }

    // Log a 21-byte GPS data chunk to the buffer (1-byte sensor ID + 4-byte timestamp + 16 bytes of GPS data)
    void log_gps_data(uint32_t timestamp, const GPS_Data& gps_data) {

      // Flush the buffer if it cannot fit 25 more bytes
      if (buffer_index + 25 > sizeof(buffer)) {
        flush_to_log();
      }
      
      // Write the sensor ID (1 byte), timestamp (4 bytes), and GPS data (16 bytes) to the buffer
      write_to_buffer(GPS_ID);                  // 1-byte sensor ID
      write_to_buffer(timestamp);               // 4-byte timestamp
      write_to_buffer(gps_data);                // 20-byte GPS data (4 bytes each for latitude, longitude, altitude, speed)
    }
    
    // Dump the buffer to the SD card
    void flush_to_log() {
      if (buffer_index > 0 && log) {
        log.write(buffer, buffer_index);
        log.flush();
        buffer_index = 0;
      }
    }
};

#endif