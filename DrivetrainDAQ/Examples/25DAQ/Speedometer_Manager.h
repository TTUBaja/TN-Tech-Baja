#ifndef SPEEDOMETER_MANAGER_H
#define SPEEDOMETER_MANAGER_H

#include <HardwareSerial.h>

// All the speedometer related code is organized here
class Speedometer_Manager {

  // Private members
  private:

    // Hardware configuration
    const int SPEEDOMETER_BAUD = 9600;
    const int LINK_TX = 2;
    const int LINK_RX = -1;
    HardwareSerial linkSerial;
  
  // Public members
  public:

    // Constructor - assign linkSerial to UART hardware port 2
    Speedometer_Manager() : linkSerial(2) {}

    // Initialize the speedometer
    void initialize() {
      linkSerial.begin(SPEEDOMETER_BAUD, SERIAL_8N1, LINK_RX, LINK_TX);
      Serial.println("Speed link serial started on TX=2.");
    }

    // Update the value of the speedometer
    void update(float speed) {
      char buffer[16];
      snprintf(buffer, sizeof(buffer), "%.2f\n", speed);
      linkSerial.print(buffer);
    }
};

#endif