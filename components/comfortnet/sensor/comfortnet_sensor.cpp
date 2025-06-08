#include "esphome/core/log.h"
#include "comfortnet_sensor.h"

namespace comfortnet {

static const char *const TAG = "comfortnet.sensor";

void ComfortnetSensor::setup() {
  this->parent_->register_listener(
    this->sensor_key_,
    [this](const ComfortnetData &datapoint) {
      if (datapoint.device_type == this->sensor_target_device_type_ || this->sensor_target_device_type_ == NodeType::ANY) {
        if (datapoint.type == ComfortnetData::DataType::FLOAT) {
          ESP_LOGV(TAG, "Callback Sensor: %s Device: 0x%02X Value: %.1f%%", this->sensor_key_.c_str(), datapoint.device_type, std::get<float>(datapoint.data));
          this->publish_state(std::get<float>(datapoint.data));
        } else {
          ESP_LOGW(TAG, "Callback Sensor: %s received wrong data type %u", datapoint.type);
        }
      }
    });
}

void ComfortnetSensor::dump_config() {
  LOG_SENSOR("", "ComfortNet Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sensor Key: %s", this->sensor_key_.c_str());
  ESP_LOGCONFIG(TAG, "  Target Device Type: %02x", this->sensor_target_device_type_);
}

}  // namespace comfortnet
