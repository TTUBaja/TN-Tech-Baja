#include "SD_Manager.h"
#include "GPS_Manager.h"
#include "RPM_Manager.h"
#include "Speedometer_Manager.h"
#include <HardwareSerial.h>


// Next steps:
// 1. Separate log files with date and times


// Problems observed at drive day:
// 1. Board repeatedly loses power (bad wiring?)
  // 31-minute stretch w/o losing power. Battery not sufficiently plugged in?
// 2. Extra pulses (interference from the engine?)
// 3. FL sensor not recording data (bad wiring?)

// 31-minute stretch ended at 5:10 pm


// Settings
const unsigned long LOG_INTERVAL = 200;

// System managers
Speedometer_Manager speedometer;
RPM_Manager rpm_sensors;
SD_Manager sd_card;
GPS_Manager gps;

// Initialization
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the system managers
  speedometer.initialize();
  rpm_sensors.initialize();
  sd_card.initialize();
  gps.initialize();
}

// The main show
void loop() {

  // State variables
  static unsigned long last_log_time = 0;

  // Transfer the contents of the RPM sensor queue to the SD card
  SensorPulse sensor_pulse;
  while(rpm_sensors.fetch_from_queue(sensor_pulse)) {
    sd_card.log_sensor_pulse(sensor_pulse.sensor_id, sensor_pulse.timestamp);
  }

  // Process incoming GPS data
  gps.empty_buffer();
  
  // Log GPS data every 500ms
  unsigned long current_time = millis();
  if (current_time - last_log_time >= LOG_INTERVAL) {
    
    // Update the RPM, MPH, and pulse counts for all sensors
    rpm_sensors.update_calculations();

    // Update the speedometer
    float speed = (rpm_sensors.FL.mph + rpm_sensors.FR.mph) / 2.0;
    speedometer.update(speed);

    // Get GPS values
    GPS_Data gps_data = gps.fetch_data();
    sd_card.log_gps_data(current_time, gps_data);

    // Debugging option - flush data buffer to the SD card every 500ms
    sd_card.flush_to_log();
    
    // Update state variables
    last_log_time = current_time;
  }
}
