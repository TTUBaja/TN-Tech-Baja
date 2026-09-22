#ifndef RPM_MANAGER_H
#define RPM_MANAGER_H

#include <HardwareSerial.h>
#include <Arduino.h>
#include "SD_Manager.h"

// RPM sensor IDs
const uint8_t FL_ID = 1;
const uint8_t FR_ID = 2;
const uint8_t R_ID = 3;
const uint8_t E_ID = 4;

// 5-byte data packet representing a sensor pulse (1-byte sensor ID + 4-byte timestamp)
struct SensorPulse {
  uint8_t sensor_id;
  uint32_t timestamp;
};

// This class manages all 4 RPM sensors
class RPM_Manager {

  // Private members
  private:

    // This queue will collect pulses from all 4 sensors
    const int QUEUE_SIZE = 150;
    QueueHandle_t output_queue;

    // This class manages 1 RPM sensor
    class RPM_Sensor {
      
      // Private members
      private:

        // All sensor pulses will be pushed to this queue
        QueueHandle_t output_queue;
        uint8_t sensor_id;

        // Hardware constants
        float mph_per_rpm;
        float gear_ratio;
        float diameter;
        int spokes;
        int pin;

        // Timing variables
        volatile unsigned long volatile_last_pulse = 0;
        volatile unsigned long volatile_pulse_interval = 0;
        volatile unsigned long volatile_pulses = 0;

        // Static wrapper for handle_pulse()
        static void handle_pulse_wrapper(void* arg) {
          RPM_Sensor* sensor = static_cast<RPM_Sensor*>(arg);
          sensor->handle_pulse();
        }

        // Handle a pulse from the sensor
        void handle_pulse() {

            // Update timing variables
            unsigned long current_time = micros();
            volatile_pulse_interval = current_time - volatile_last_pulse;
            volatile_last_pulse = current_time;
            volatile_pulses++;

            // Push the sensor pulse to the output queue
            SensorPulse sensor_pulse = {sensor_id, current_time};
            xQueueSendFromISR(output_queue, &sensor_pulse, NULL);
        }
      
      // Public members
      public:

        // Outputs
        unsigned long pulses = 0;
        float rpm = 0;
        float mph = 0;

        // Constructor (set hardware constants)
        RPM_Sensor(uint8_t set_id, float set_diameter, int set_spokes, int set_pin, float set_gear_ratio) : sensor_id(set_id), diameter(set_diameter), spokes(set_spokes), pin(set_pin), gear_ratio(set_gear_ratio) {
          mph_per_rpm = ((PI * diameter) * 60.0) / (12.0 * 5280.0);
        }

        // Initialize the physical sensor
        void initialize(QueueHandle_t q) {
            output_queue = q;
            pinMode(pin, INPUT_PULLUP);
            attachInterruptArg(digitalPinToInterrupt(pin), handle_pulse_wrapper, this, RISING);
        }

        // Update the RPM, MPH, and pulse count
        void update_calculations() {
          pulses = volatile_pulses;
          float freq = volatile_pulse_interval > 0 ? 1000000.0 / volatile_pulse_interval : 0;
          rpm = ((freq * 60.0) / spokes) / gear_ratio;
          mph = mph_per_rpm * rpm;
        }
    };
  
  // Public members
  public:

    // RPM sensors
    RPM_Sensor FL;
    RPM_Sensor FR;
    RPM_Sensor R;
    RPM_Sensor E;

    // Constructor      sensor_id   diameter    spokes    pin     gear_ratio
    RPM_Manager() : FL( FL_ID,      22.0,       8,        14,     1.0),
                    FR( FR_ID,      22.0,       8,        33,     1.0),
                    R(  R_ID,       22.0,       10,       32,     6.42),
                    E(  E_ID,       0.0,        6,        16,     1.0) {}

    // Initialize all sensors and create the output queue
    void initialize() {

      // Create the output queue
      output_queue = xQueueCreate(QUEUE_SIZE, sizeof(SensorPulse));

      // Initialize each sensor
      FL.initialize(output_queue);
      FR.initialize(output_queue);
      R.initialize(output_queue);
      E.initialize(output_queue);
    }

    // Update RPM, MPH, and pulse counts for all sensors
    void update_calculations() {

      // Temporarily disable interrupts while reading shared variables
      noInterrupts();
      FL.update_calculations();
      FR.update_calculations();
      R.update_calculations();
      E.update_calculations();
      interrupts();
    }
    
    // Returns true if there is data in the queue, false if it is empty
    bool fetch_from_queue(SensorPulse& event) {
      return xQueueReceive(output_queue, &event, 0) == pdTRUE;
    }
};

#endif