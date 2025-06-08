#include "esphome/core/log.h"
#include "comfortnet_binary_sensor.h"

namespace comfortnet {

static const char *const TAG = "comfortnet.binary_sensor";

void ComfortnetBinarySensor::setup() {
  this->parent_->register_listener(
    this->sensor_key_,
    [this](const ComfortnetData &datapoint) {
      if (datapoint.device_type == this->sensor_target_device_type_ || this->sensor_target_device_type_ == NodeType::ANY) {
        if (datapoint.type == ComfortnetData::DataType::BOOLEAN) {
          ESP_LOGV(TAG, "Callback BinarySensor: %s Device: 0x%02X Value: %s", this->sensor_key_.c_str(), datapoint.device_type, std::get<bool>(datapoint.data) ? "true" : "false");
          this->publish_state(std::get<bool>(datapoint.data));
        } else {
          ESP_LOGW(TAG, "Callback BinarySensor: %s received wrong data type %u", datapoint.type);
        }
      }
    });
}

void ComfortnetBinarySensor::dump_config() {
  ESP_LOGCONFIG(TAG, "ComfortNet Binary Sensor:");
  ESP_LOGCONFIG(TAG, "  Sensor Key: %s", this->sensor_key_.c_str());
  ESP_LOGCONFIG(TAG, "  Target Device Type: %02x", this->sensor_target_device_type_);
}

}  // namespace comfortnet
