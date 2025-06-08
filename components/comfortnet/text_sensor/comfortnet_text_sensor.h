#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "../comfortnet.h"

namespace esphome {
namespace comfortnet {

class ComfortnetTextSensor : public text_sensor::TextSensor, public Component, public ComfortnetClient {
 public:
  void setup() override;
  void dump_config() override;
  void set_sensor_id(const std::string &sensor_id) { this->sensor_id_ = sensor_id; }

 protected:
  std::string sensor_id_{""};
};

}  // namespace comfortnet
}  // namespace esphome
