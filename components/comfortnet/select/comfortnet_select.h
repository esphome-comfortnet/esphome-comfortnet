#pragma once

#include "esphome/core/component.h"
#include "esphome/components/select/select.h"
#include "../comfortnet.h"

#include <vector>

namespace esphome {
namespace comfortnet {

class ComfortnetSelect : public select::Select, public Component, public ComfortnetClient {
 public:
  void setup() override;
  void dump_config() override;
  void set_select_id(const std::string &select_id) { this->select_id_ = select_id; }
  void set_select_mappings(std::vector<uint8_t> mappings) { this->mappings_ = std::move(mappings); }

 protected:
  void control(const std::string &value) override;

  std::string select_id_{""};
  std::vector<uint8_t> mappings_;
};

}  // namespace comfortnet
}  // namespace esphome
