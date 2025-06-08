#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../comfortnet.h"

namespace esphome {
namespace comfortnet {

class ComfortnetSwitch : public switch_::Switch, public Component, public ComfortnetClient {
 public:
  void setup() override;
  void dump_config() override;
  void set_switch_id(const std::string &switch_id) { this->switch_id_ = switch_id; }

 protected:
  void write_state(bool state) override;

  std::string switch_id_{""};
};

}  // namespace comfortnet
}  // namespace esphome
