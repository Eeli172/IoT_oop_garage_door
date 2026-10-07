#ifndef IOT_OOP_GARAGE_DOOR_MACROS_H
#define IOT_OOP_GARAGE_DOOR_MACROS_H


// TIMEOUTS
#define INTERRUPT_TIMEOUT_MS 10
#define WIFI_CONNECTING_TIMEOUT_MS 30000
#define CHECK_WIFI_STATUS_INTERVAL_SECONDS 10
#define LED_BLINK_TIMEOUT_MS 500
#define I2C_POLLING_TIMEOUT_US 100
#define EEPROM_ACK_TIMEOUT_MS 15
#define MOTOR_STEP_DELAY_US 1000

// CALIBRATION
#define STUCK_STEP_TOLERANCE 200 //safe minimum so it will not stop randomly 150

// MQTT
#define MQTT_BROKER_PORT 1883
#define MQTT_CLIENT_ID "rp2040_gd"
#define MQTT_CLIENT_KA_TIME 60
#define MQTT_DEFAULT_MSG "<Initialization complete>"
#define MQTT_TOPIC_RUN_INSTRUCTIONS "garage/door/run_instructions"
#define MQTT_TOPIC_INSTRUCTION_RESPONSE "garage/door/response"
#define MQTT_TOPIC_DOOR_STATE "garage/door/door_status"
#define MQTT_TOPIC_SYSTEM_STATE "garage/door/system_state"
#define MQTT_TOPIC_ERROR_STATE "garage/door/error_state"

// EEPROM
#define EEPROM_ADDR_CALIBRATION_STATE 0x0000 // 0 - 1 (1 byte + inverted)
#define EEPROM_ADDR_DOOR_STATE 0x0002 // 2 - 3 (1 byte + inverted)
#define EEPROM_ADDR_STEPS_PER_ENCODER 0x0004 // 4 - 11 (4 bytes + inverted)
#define EEPROM_ADDR_DIRECTION 0x000C // 12 - 13 (1 byte + inverted)
#define EEPROM_ADDR_MANUALSTOP 0x000E // 14 - 15 (1 byte + inverted)

#define EEPROM_ADDR_BYTES 2 
#define EEPROM_BUFFER_SIZE_BYTE (BYTE * 2) // (data + ~data)
#define EEPROM_BUFFER_SIZE_FLOAT (FLOAT_BYTES * 2) // (data + ~data)
#define EEPROM_PAYLOAD_SIZE_BYTE (EEPROM_ADDR_BYTES + EEPROM_BUFFER_SIZE_BYTE) // (addr + (data + ~data))
#define EEPROM_PAYLOAD_SIZE_FLOAT (EEPROM_ADDR_BYTES + EEPROM_BUFFER_SIZE_FLOAT) // (addr + (data + ~data))

// I2C
#define I2C_ADDR_EEPROM 0x50
#define I2C_PORT i2c1
#define I2C_FREQ 100000 // 100 kHz

// UNIVERSAL
#define IP_OCTETS 4
#define BYTE 1
#define BYTE_BITS 8
#define FLOAT_BYTES sizeof(float)



/* GPIOS */
// BUTTONS
#define PIN_SW0 9
#define PIN_SW1 8
#define PIN_SW2 7
#define SW_AMOUNT 3

// LIMIT SWITCHES (GARAGE DOOR)
#define PIN_LMSW_MTR_OPEN 16 
#define PIN_LMSW_ENC_CLOSED 17 

// ENCODERS
#define PIN_ENCODER_A 28
#define PIN_ENCODER_B 27

// I2C
#define PIN_I2C_SDA 14
#define PIN_I2C_SCL 15

// MOTOR COIL GPIOS
#define PIN_COIL_1 2
#define PIN_COIL_2 3
#define PIN_COIL_3 6
#define PIN_COIL_4 13
#define MTR_COILS 4
#define MTR_STEPS 8

// LED GPIOS
#define PIN_LED_1 20
#define PIN_LED_2 21
#define PIN_LED_3 22
#define LEDS_AMOUNT 3


#endif //IOT_OOP_GARAGE_DOOR_MACROS_H