#pragma once

#include <cinttypes>
#include <vector>
#ifdef ARDUINO
#include <Arduino.h>
#else
#include "esp_random.h"
#include "esp_mac.h"
#endif

namespace comfortnet {

/**
 * Defined in ClimateTalk Alliance CT2.0 CT-485 Networking Specification Revision 01
 * 6.1 Node Address
 */
enum class NodeAddress : uint8_t {
  BROADCAST = 0x00,                /* Broadcast */
  COORDINATION_ARBITRATION = 0xFE, /* Coordinatior Arbitration*/
  COORDINATOR = 0xFF,              /* Network Coordinator */
};

/**
 * Defined in ClimateTalk Alliance CT2.0 CT-485 Data Link Specification Revision 01
 * 6.3 Subnets
 */
enum class Subnet : uint8_t {
  BROADCAST = 0x00,
  MAINTENANCE = 0x01,
  VERSION_1 = 0x02,
  VERSION_2 = 0x03,
};

/**
 * Defined in ClimateTalk Alliance CT2.0 CT-485 Networking Specification Revision 01
 * 6.3 Send Methods
 */
enum class SendMethod : uint8_t {
  NO_ROUTE = 0x00,        /* 6.3.1 - Talking to direct neighbors */
  CONTROL_COMMAND = 0x01, /* 6.3.2 - Send to coordinator, which forwards it along. New destination is the "best" node
                             for the desired control command (stored in send parameter 1) */
  NODE_TYPE = 0x02, /* 6.3.3 - Send to coordinator, which forwards it along. New destination is the "best" node of a
                       given type (stored in send parameter 1) */
  NODE_ID = 0x03,   /* 6.3.4 - Send to coordinator, which forwards it along. New destination is the node at the desired
                      position (stored in send parameter 1) in the node list */
};

/**
 * Defined in ClimateTalk Alliance CT2.0 CT-485 Networking Specification Revision 01
 * 6.3.2 Send Method 1: Routing by Priority Control Command Device
 *
 * Control command options to be used with SendMethod::CONTROL_COMMAND
 */
enum class SendMethodControlCommand : uint8_t {
  HEAT = 0x64,
  COOL = 0x65,
  FAN = 0x66,
  EMERGENCY = 0x67,
  DEFROST = 0x68,
  AUX_HEAT = 0x69,
};

/**
 * Defined in ClimateTalk Alliance CT2.0 Command Reference Revision 00
 * Annex B Node Type Table
 */
enum class NodeType : uint8_t {
  ANY = 0x00,                      /* Any (For addressing purposes, not a valid type) */
  THERMOSTAT = 0x01,               /* Thermostat */
  GAS_FURNACE = 0x02,              /* Gas Furnace */
  AIR_HANDLER = 0x03,              /* Air Handler */
  AIR_CONDITIONER = 0x04,          /* Air Conditioner */
  HEAT_PUMP = 0x05,                /* Heat Pump */
  ELECTRIC_FURNACE = 0x06,         /* Electric Furnace */
  PACKAGE_SYSTEM_GAS = 0x07,       /* Package System - Gas */
  PACKAGE_SYSTEM_ELECTRIC = 0x08,  /* Package System - Electric */
  CROSSOVER = 0x09,                /* Crossover (aka OBBI) */
  SECONDARY_COMPRESSOR = 0x0A,     /* Secondary Compressor */
  AIR_EXCHANGER = 0x0B,            /* Air Exchanger */
  UNITARY_CONTROL = 0x0C,          /* Unitary Control */
  DEHUMIDIFIER = 0x0D,             /* Dehumidifier */
  ELECTRONIC_AIR_CLEANER = 0x0E,   /* Electronic Air Cleaner */
  ERV = 0x0F,                      /* ERV */
  HUMIDIFIER_EVAPORATIVE = 0x10,   /* Humidifier (Evaporative) */
  HUMIDIFIER_STEAM = 0x11,         /* Humidifier (Steam) */
  HRV = 0x12,                      /* HRV */
  IAQ_ANALYZER = 0x13,             /* IAQ Analyzer */
  MEDIA_AIR_CLEANER = 0x14,        /* Media Air Cleaner */
  ZONE_CONTROL = 0x15,             /* Zone Control */
  ZONE_USER_INTERFACE = 0x16,      /* Zone User Interface */
  BOILER = 0x17,                   /* Boiler */
  WATER_HEATER_GAS = 0x18,         /* Water Heater – Gas */
  WATER_HEATER_ELECTRIC = 0x19,    /* Water Heater – Electric */
  WATER_HEATER_COMMERCIAL = 0x1A,  /* Water Heater - Commercial */
  POOL_HEATER = 0x1B,              /* Pool Heater */
  CEILING_FAN = 0x1C,              /* Ceiling Fan */
  GATEWAY = 0x1D,                  /* Gateway */
  DIAGNOSTIC_DEVICE = 0x1E,        /* Diagnostic Device */
  LIGHTING_CONTROL = 0x1F,         /* Lighting Control */
  SECURITY_SYSTEM = 0x20,          /* Security System */
  UV_LIGHT = 0x21,                 /* UV Light */
  WEATHER_DATA_DEVICE = 0x22,      /* Weather Data Device */
  WHOLE_HOUSE_FAN = 0x23,          /* Whole House Fan */
  SOLAR_INVERTER = 0x24,           /* Solar Inverter */
  ZONE_DAMPER = 0x25,              /* Zone Damper */
  ZONE_TEMPERATURE_CONTROL = 0x26, /* Zone Temperature Control (ZTC) */
  TEMPERATURE_SENSOR = 0x27,       /* Temperature Sensor */
  OCCUPANCY_SENSOR = 0x28,         /* Occupancy Sensor */
  NETWORK_COORDINATOR = 0xA5,      /* Network Coordinator */
};

/**
 * Defined in ClimateTalk Alliance CT2.0 API Reference Revision 01
 * 5.0 CT-CIM Mapped Message Reference
 */
enum class MessageType : uint8_t {
  GET_CONFIGURATION = 0x01,                               /* Get Configuration */
  GET_CONFIGURATION_RESPONSE = 0x81,                      /* Get Configuration Response */
  GET_STATUS = 0x02,                                      /* Get Status */
  GET_STATUS_RESPONSE = 0x82,                             /* Get Status Response */
  SET_CONTROL_COMMAND = 0x03,                             /* Set Control Command */
  SET_CONTROL_COMMAND_RESPONSE = 0x83,                    /* Set Control Command Response */
  SET_DISPLAY_MESSAGE = 0x04,                             /* Set Display Message */
  SET_DISPLAY_MESSAGE_RESPONSE = 0x84,                    /* Set Display Message Response */
  SET_DIAGNOSTICS = 0x05,                                 /* Set Diagnostics */
  SET_DIAGNOSTICS_RESPONSE = 0x85,                        /* Set Diagnostics Response */
  GET_DIAGNOSTICS = 0x06,                                 /* Get Diagnostics */
  GET_DIAGNOSTICS_RESPONSE = 0x86,                        /* Get Diagnostics Response */
  GET_SENSOR_DATA = 0x07,                                 /* Get Sensor Data */
  GET_SENSOR_DATA_RESPONSE = 0x87,                        /* Get Sensor Data Response */
  SET_IDENTIFICATION = 0x0D,                              /* Set Identification */
  SET_IDENTIFICATION_RESPONSE = 0x8D,                     /* Set Identification Response */
  GET_IDENTIFICATION = 0x0E,                              /* Get Identification */
  GET_IDENTIFICATION_RESPONSE = 0x8E,                     /* Get Identification Response */
  SET_APPLICATION_SHARED_DATA_TO_NETWORK = 0x10,          /* Set Application Shared Data to Network */
  SET_APPLICATION_SHARED_DATA_TO_NETWORK_RESPONSE = 0x90, /* Set Application Shared Data to Network Response */
  GET_APPLICATION_SHARED_DATA_TO_NETWORK = 0x11,          /* Get Application Shared Data to Network */
  GET_APPLICATION_SHARED_DATA_TO_NETWORK_RESPONSE = 0x91, /* Get Application Shared Data to Network Response */
  SET_MANUFACTURER_DEVICE_DATA = 0x12,                    /* Set Manufacturer Device Data */
  SET_MANUFACTURER_DEVICE_DATA_RESPONSE = 0x92,           /* Set Manufacturer Device Data Response */
  GET_MANUFACTURER_DEVICE_DATA = 0x13,                    /* Get Manufacturer Device Data */
  GET_MANUFACTURER_DEVICE_DATA_RESPONSE = 0x93,           /* Get Manufacturer Device Data Response */
  SET_NETWORK_NODE_LIST = 0x14,                           /* Set Network Node List */
  SET_NETWORK_NODE_LIST_RESPONSE = 0x94,                  /* Set Network Node List Response */
  DIRECT_MEMORY_ACCESS_READ = 0x1D,                       /* Direct Memory Access Read */
  DIRECT_MEMORY_ACCESS_READ_RESPONSE = 0x9D,              /* Direct Memory Access Read Response */
  DIRECT_MEMORY_ACCESS_READ_RESPONSE_MOTOR = 0x1E,        /* Direct Memory Access Read Response Motor */
  SET_MANUFACTURER_GENERIC_DATA = 0x1F,                   /* Set Manufacturer Generic Data */
  SET_MANUFACTURER_GENERIC_DATA_RESPONSE = 0x9F,          /* Set Manufacturer Generic Data Response */
  GET_MANUFACTURER_GENERIC_DATA = 0x20,                   /* Get Manufacturer Generic Data */
  GET_MANUFACTURER_GENERIC_DATA_RESPONSE = 0xA0,          /* Get Manufacturer Generic Data Response */
  GET_MANUFACTURER_GENERIC_DATA_RESPONSE_MOTOR = 0x21,    /* Get Manufacturer Generic Data Response Motor */
  GET_USER_MENU = 0x41,                                   /* Get User Menu */
  GET_USER_MENU_RESPONSE = 0xC1,                          /* Get User Menu Response */
  SET_USER_MENU = 0x42,                                   /* Set User Menu */
  SET_USER_MENU_RESPONSE = 0xC2,                          /* Set User Menu Response */
  SET_FACTORY_SHARED_DATA_TO_APPLICATION = 0x43,          /* Set Factory Shared Data to Application */
  SET_FACTORY_SHARED_DATA_TO_APPLICATION_RESPONSE = 0xC3, /* Set Factory Shared Data to Application Response */
  GET_SHARED_DATA_FROM_APPLICATION = 0x44,                /* Get Shared Data from Application */
  GET_SHARED_DATA_FROM_APPLICATION_RESPONSE = 0xC4,       /* Get Shared Data from Application Response */
  SET_ECHO_DATA = 0x5A,                                   /* Set Echo Data */
  SET_ECHO_DATA_RESPONSE = 0xDA,                          /* Set Echo Data Response */
  /**
   * CT-485 Specific Messages
   * Defined in ClimateTalk Alliance CT2.0 API Reference Revision 01
   * Table 68 CT-485 Message Types
   */
  REQUEST_TO_RECEIVE_RESPONSE = 0x00,    /* Request to Receive/Response */
  NETWORK_STATE_REQUEST = 0x75,          /* Network State Request */
  NETWORK_STATE_REQUEST_RESPONSE = 0xF5, /* Network State Request Response */
  ADDRESS_CONFIRMATION = 0x76,           /* Address Confirmation */
  ADDRESS_CONFIRMATION_RESPONSE = 0xF6,  /* Address Confirmation Response */
  TOKEN_OFFER = 0x77,                    /* Token Offer */
  TOKEN_OFFER_RESPONSE = 0xF7,           /* Token Offer Response */
  VERSION_ANNOUNCEMENT = 0x78,           /* Version Announcement */
  NODE_DISCOVERY = 0x79,                 /* Node Discovery */
  NODE_DISCOVERY_RESPONSE = 0xF9,        /* Node Discovery Response */
  SET_ADDRESS = 0x7A,                    /* Set Address */
  SET_ADDRESS_RESPONSE = 0xFA,           /* Set Address Response */
  GET_NODE_ID = 0x7B,                    /* Get Node ID */
  GET_NODE_ID_RESPONSE = 0xFB,           /* Get Node ID Response */
  NETWORK_SHARED_DATA_SECTOR_IMAGE_READ_WRITE_REQUEST =
      0x7D, /* Network Shared Data Sector Image Read / Write Request Format */
  NETWORK_SHARED_DATA_SECTOR_IMAGE_READ_WRITE_REQUEST_RESPONSE =
      0xFD,                                     /* Network Shared Data Sector Image Read / Write Response */
  NETWORK_ENCAPSULATION_REQUEST = 0x7E,         /* Network Encapsulation Request */
  NETWORK_ENCAPSULATION_REQUEST_RESPONSE = 0xFE /* Network Encapsulation Request Response */
};

#define PACKET_RESPONSE(packet) static_cast<MessageType>(static_cast<uint8_t>(packet) | 0x80)
#define PACKET_REQUEST(packet) static_cast<MessageType>(static_cast<uint8_t>(packet) & ~0x80)
#define PACKET_NUMBER(isDataFlow, isVersion1) ((isDataFlow) ? 0x80 : 0x00) | ((isVersion1) ? 0x20 : 0x00)
#define PACKET_IS_DATAFLOW(packetNumber) ((packetNumber & 0x80) > 0)

#define R2R_ACK 0x06
#define R2R_NACK 0x15

#define MAX_PAYLOAD_SIZE 240

enum class CommandType : uint16_t {
  HEAT_SET_POINT_TEMPERATURE_MODIFY = 0x01,    /* Heat Set Point Temperature Modify */
  COOL_SET_POINT_TEMPERATURE_MODIFY = 0x02,    /* Cool Set Point Temperature Modify */
  HEAT_PROFILE_CHANGE = 0x03,                  /* Heat Profile Change */
  COOL_PROFILE_CHANGE = 0x04,                  /* Cool Profile Change */
  SYSTEM_SWITCH_MODIFY = 0x05,                 /* System Switch Modify */
  PERMANENT_SET_POINT_TEMP_HOLD_MODIFY = 0x06, /* Permanent Set Point Temp & Hold Modify */
  FAN_KEY_SELECTION = 0x07,                    /* Fan Key Selection */
  HOLD_OVERRIDE = 0x08,                        /* Hold Override */
  BEEPER_ENABLE = 0x09,                        /* Beeper Enable */
  FAHRENHEIT_CELSIUS_DISPLAY = 0x0C,           /* Fahrenheit/Celsius Display */
  COMFORT_RECOVERY_MODIFY = 0x0E,              /* Comfort Recovery (EMR) Modify */
  REAL_TIME_DAY_OVERRIDE = 0x0F,               /* Real Time/Day Override */
  CHANGE_FILTER_TIME_REMAINING = 0x14,         /* Change Filter Time Remaining */
  VACATION_MODE = 0x15,                        /* Vacation Mode */
  HIGH_ALARM_LIMIT_CHANGE = 0x16,              /* High Alarm Limit Change */
  LOW_ALARM_LIMIT_CHANGE = 0x17,               /* Low Alarm Limit Change */
  HIGH_OUTDOOR_ALARM_LIMIT_CHANGE = 0x18,      /* High Outdoor Alarm Limit Change */
  LOW_OUTDOOR_ALARM_LIMIT_CHANGE = 0x19,       /* Low Outdoor Alarm Limit Change */
  TEMP_DISPLAY_ADJ_FACTOR_CHANGE = 0x1A,       /* Temp Display Adjustment Factor Change */
  CLEAR_COMPRESSOR_RUN_TIME = 0x2D,            /* Clear Compressor Run Time */
  RESET_CONTROL = 0x31,                        /* Reset Control */
  COMPRESSOR_LOCKOUT = 0x33,                   /* Compressor Lockout */
  HOLD_RELEASE = 0x3D,                         /* Hold Release */
  PROGRAM_INTERVAL_TYPE_MODIFICATION = 0x3E,   /* Program Interval Type Modification */
  COMMUNICATIONS_RECEIVER_ON_OFF = 0x3F,       /* Communications Receiver On/Off */
  FORCE_PHONE_NUMBER_DISPLAY = 0x40,           /* Force Phone Number Display */
  RESTORE_FACTORY_DEFAULTS = 0x45,             /* Restore Factory Defaults */
  CUSTOM_MESSAGE_AREA_DISPLAY_DATA = 0x46,     /* Custom Message Area Display Data */
  SET_POINT_TEMP_AND_TEMPORARY_HOLD = 0x47,    /* Set Point Temp and Temporary Hold */
  CONTINUOUS_DISPLAY_LIGHT = 0x48,             /* Continuous Display Light */
  ADVANCE_REAL_TIME_DAY_OVERRIDE = 0x4E,       /* Advance Real Time/Day Override */
  KEYPAD_LOCKOUT = 0x4F,                       /* Keypad Lockout */
  TEST_MODE = 0x50,                            /* Test Mode */
  SUBSYSTEM_INSTALLATION_TEST = 0x51,          /* Subsystem Installation Test */
  SET_POINT_TEMP_TEMPORARY_HOLD = 0x53,        /* Set Point Temp and Temporary Hold */
  COMFORT_MODE_MODIFICATION = 0x55,            /* Comfort Mode Modification */
  LIMITED_HEAT_AND_COOL_RANGE = 0x56,          /* Limited Heat and Cool Range */
  AUTO_PAIRING_REQUEST = 0x57,                 /* Auto-Pairing Request */
  PAIRING_OWNERSHIP_REQUEST = 0x58,            /* Pairing Ownership Request */
  REVERSING_VALVE_CONFIG = 0x59,               /* Reversing Valve Config */
  DEHUM_HUM_CONFIG = 0x5A,                     /* Dehumidification/Humidification Config */
  CHANGE_UV_LIGHT_MAINTENANCE_TIMER = 0x5B,    /* Change UV Light Maintenance Timer */
  CHANGE_HUMIDIFIER_PAD_MAINT_TIMER = 0x5C,    /* Change Humidifier Pad Maintenance Timer */
  DEHUMIDIFICATION_SET_POINT_MODIFY = 0x5D,    /* Dehumidification Set Point Modify */
  HUMIDIFICATION_SET_POINT_MODIFY = 0x5E,      /* Humidification Set Point Modify */
  DAMPER_CLOSURE_POSITION_DEMAND = 0x60,       /* Damper Closure Position Demand */
  SUBSYSTEM_BUSY_STATUS = 0x61,                /* Subsystem Busy Status */
  DEHUMIDIFICATION_DEMAND = 0x62,              /* Dehumidification Demand */
  HUMIDIFICATION_DEMAND = 0x63,                /* Humidification Demand */
  HEAT_DEMAND = 0x64,                          /* Heat Demand */
  COOL_DEMAND = 0x65,                          /* Cool Demand */
  FAN_DEMAND = 0x66,                           /* Fan Demand */
  BACK_UP_HEAT_DEMAND = 0x67,                  /* Back-Up Heat Demand */
  DEFROST_HEAT_DEMAND = 0x68,                  /* Defrost Heat Demand */
  AUX_HEAT_DEMAND = 0x69,                      /* Aux/Alt Heat Demand */
  SET_MOTOR_SPEED = 0x6A,                      /* Set Motor Speed */
  SET_MOTOR_TORQUE = 0x6B,                     /* Set Motor Torque */
  SET_AIRFLOW_DEMAND = 0x6C,                   /* Set Airflow Demand */
  SET_CONTROL_MODE = 0x6D,                     /* Set Control Mode */
  SET_DEMAND_RAMP_RATE = 0x6E,                 /* Set Demand Ramp Rate */
  SET_MOTOR_DIRECTION = 0x6F,                  /* Set Motor Direction */
  SET_MOTOR_TORQUE_PERCENT = 0x70,             /* Set Motor Torque Percent */
  SET_MOTOR_POSITION_DEMAND = 0x71,            /* Set Motor Position Demand */
  SET_BLOWER_COEFFICIENT_1 = 0x72,             /* Set Blower Coefficient 1 */
  SET_BLOWER_COEFFICIENT_2 = 0x73,             /* Set Blower Coefficient 2 */
  SET_BLOWER_COEFFICIENT_3 = 0x74,             /* Set Blower Coefficient 3 */
  SET_BLOWER_COEFFICIENT_4 = 0x75,             /* Set Blower Coefficient 4 */
  SET_BLOWER_COEFFICIENT_5 = 0x76,             /* Set Blower Coefficient 5 */
  SET_BLOWER_IDENTIFICATION_0 = 0x77,          /* Set Blower Identification 0 */
  SET_BLOWER_IDENTIFICATION_1 = 0x78,          /* Set Blower Identification 1 */
  SET_BLOWER_IDENTIFICATION_2 = 0x79,          /* Set Blower Identification 2 */
  SET_BLOWER_IDENTIFICATION_3 = 0x7A,          /* Set Blower Identification 3 */
  SET_BLOWER_IDENTIFICATION_4 = 0x7B,          /* Set Blower Identification 4 */
  SET_BLOWER_IDENTIFICATION_5 = 0x7C,          /* Set Blower Identification 5 */
  SET_SPEED_LIMIT = 0x7F,                      /* Set Speed Limit */
  SET_TORQUE_LIMIT = 0x80,                     /* Set Torque Limit */
  SET_AIRFLOW_LIMIT = 0x81,                    /* Set Airflow Limit */
  SET_POWER_OUTPUT_LIMIT = 0x82,               /* Set Power Output Limit */
  SET_DEVICE_TEMPERATURE_LIMIT = 0x83,         /* Set Device Temperature Limit */
  STOP_MOTOR_BY_BRAKING = 0x85,                /* Stop Motor by Braking */
  RUN_STOP_MOTOR = 0x86,                       /* Run/Stop Motor */
  SET_DEMAND_RAMP_TIME = 0x88,                 /* Set Demand Ramp Time */
  SET_INDUCER_RAMP_RATE = 0x89,                /* Set Inducer Ramp Rate */
  SET_BLOWER_COEFFICIENT_6 = 0x8A,             /* Set Blower Coefficient 6 */
  SET_BLOWER_COEFFICIENT_7 = 0x8B,             /* Set Blower Coefficient 7 */
  SET_BLOWER_COEFFICIENT_8 = 0x8C,             /* Set Blower Coefficient 8 */
  SET_BLOWER_COEFFICIENT_9 = 0x8D,             /* Set Blower Coefficient 9 */
  SET_BLOWER_COEFFICIENT_10 = 0x8E,            /* Set Blower Coefficient 10 */
  PUBLISH_PRICE = 0xE0,                        /* Publish Price */
  WATER_HEATER_MODIFY = 0xF0,                  /* Water Heater Modify */
};

#define MAC_ADDRESS_SIZE 8
#define MAC_ADDRESS_RESERVED_POS 0x00

struct MacAddress {
  uint8_t mac[MAC_ADDRESS_SIZE];
  void setRandom() {
    // TODO Random MACs are okay, but really should be persisted across restarts...
#ifdef ARDUINO
    for (int i = 1; i < MAC_ADDRESS_SIZE; i++) {  // We don't need to set the first byte
      mac[i] = random(0x00, 0xFF);                // Just generate a random MAC for now...
    }
    mac[MAC_ADDRESS_RESERVED_POS] = 0xFF;  // Non-zero value flags a random MAC
#else
    if (esp_mac_addr_len_get(esp_mac_type_t::ESP_MAC_IEEE802154) == 8) {
      esp_efuse_mac_get_default(mac);        // Copy hardware efuse MAC
      mac[MAC_ADDRESS_RESERVED_POS] = 0x00;  // Mark this MAC as not random
    } else if (esp_mac_addr_len_get(esp_mac_type_t::ESP_MAC_EFUSE_FACTORY) == 6) {
      esp_efuse_mac_get_default(mac + 2);           // Copy hardware efuse MAC, but skip first 2 bytes
      mac[MAC_ADDRESS_RESERVED_POS] = 0x00;         // Mark this MAC as not random
      mac[MAC_ADDRESS_RESERVED_POS + 1] = 0x00;     // Set the remaining byte to 0x00 too for fun I guess, no reason
    } else {                                        // No MAC available, just generate a random one for now...
      for (int i = 1; i < MAC_ADDRESS_SIZE; i++) {  // We don't need to set the first byte
        mac[i] = esp_random() & 0xFF;
      }
      mac[MAC_ADDRESS_RESERVED_POS] = 0xFF;  // Non-zero value flags a random MAC
    }
#endif
  }
  void write(std::vector<uint8_t> &data) { data.insert(data.end(), mac, mac + MAC_ADDRESS_SIZE); };
};

#undef MAC_ADDRESS_RESERVED_POS

#define SESSION_ID_SIZE 8

struct SessionId {
  uint8_t sessionid[SESSION_ID_SIZE];
  void clear() {
    for (uint8_t i = 0; i < SESSION_ID_SIZE; i++) {
      sessionid[i] = 0;
    }
  }
  void setRandom() {
    for (int i = 0; i < SESSION_ID_SIZE; i++) {
#ifdef ARDUINO
      sessionid[i] = random(0x00, 0xFF);
#else
      sessionid[i] = esp_random() & 0xFF;
#endif
    }
  }
  void write(std::vector<uint8_t> &data) { data.insert(data.end(), sessionid, sessionid + SESSION_ID_SIZE); };
};

}  // namespace comfortnet
