#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../comfortnet.h"

namespace comfortnet {

class ComfortnetBinarySensor : public esphome::binary_sensor::BinarySensor, public esphome::Component, public ComfortnetClient {
 public:
  void setup() override;
  void dump_config() override;
  void set_sensor_key(const std::string &sensor_key) { this->sensor_key_ = sensor_key; };
  void set_sensor_target_device_type(uint8_t type) { this->sensor_target_device_type_ = static_cast<NodeType>(type); };

 protected:
  std::string sensor_key_{""};
  NodeType sensor_target_device_type_{NodeType::ANY};

};

}  // namespace comfortnet
