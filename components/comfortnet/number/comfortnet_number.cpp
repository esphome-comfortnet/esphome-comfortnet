#include "comfortnet_number.h"
#include "esphome/core/log.h"

namespace comfortnet {

static const char *const TAG = "comfortnet.number";

void ComfortnetNumber::setup() {
  this->parent_->register_listener(
    this->sensor_key_,
    [this](const ComfortnetData &datapoint) {
      if (datapoint.device_type == this->sensor_target_device_type_ || this->sensor_target_device_type_ == NodeType::ANY) {
        if (datapoint.type == ComfortnetData::DataType::FLOAT) {
          ESP_LOGV(TAG, "Callback Number: %s Device: 0x%02X Value: %.1f%%", this->sensor_key_.c_str(), datapoint.device_type, std::get<float>(datapoint.data));
          this->publish_state(std::get<float>(datapoint.data));
        } else {
          ESP_LOGW(TAG, "Callback Number: %s received wrong data type %u", datapoint.type);
        }
      }
    });
}

void ComfortnetNumber::control(float value) {
  ESP_LOGV(TAG, "Setting number %s: %f", this->number_id_.c_str(), value);
  if (this->type_ == ComfortnetDatapointType::FLOAT) {
    this->parent_->set_float_datapoint_value(this->number_id_, value, this->src_adr_);
  } else if (this->type_ == ComfortnetDatapointType::ENUM_TEXT) {
    this->parent_->set_enum_datapoint_value(this->number_id_, value, this->src_adr_);
  }
  this->publish_state(value);
}

void ComfortnetNumber::dump_config() {
  LOG_NUMBER("", "ComfortNet Number", this);
  ESP_LOGCONFIG(TAG, "  Sensor Key: %s", this->sensor_key_.c_str());
  ESP_LOGCONFIG(TAG, "  Target Device Type: %02x", this->sensor_target_device_type_);
}

}  // namespace comfortnet
