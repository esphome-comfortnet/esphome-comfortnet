#include "esphome/core/log.h"
#include "comfortnet_switch.h"

namespace esphome {
namespace comfortnet {

static const char *const TAG = "comfortnet.switch";

void ComfortnetSwitch::setup() {
  this->parent_->register_listener(
      this->switch_id_, this->request_mod_, this->request_once_,
      [this](const ComfortnetDatapoint &datapoint) {
        ESP_LOGV(TAG, "MCU reported switch %s is: %s", this->switch_id_.c_str(), ONOFF(datapoint.value_enum));
        this->publish_state(datapoint.value_enum);
      },
      false, this->src_adr_);
}

void ComfortnetSwitch::write_state(bool state) {
  ESP_LOGV(TAG, "Setting switch %s: %s", this->switch_id_.c_str(), ONOFF(state));
  this->parent_->set_enum_datapoint_value(this->switch_id_, state, this->src_adr_);
  this->publish_state(state);
}

void ComfortnetSwitch::dump_config() {
  LOG_SWITCH("", "ComfortNet Switch", this);
  ESP_LOGCONFIG(TAG, "  Switch has datapoint ID %s", this->switch_id_.c_str());
}

}  // namespace comfortnet
}  // namespace esphome
