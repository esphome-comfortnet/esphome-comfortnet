#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "comfortnet.h"

#include <vector>

namespace comfortnet {

class ComfortnetCommandTrigger : public esphome::Trigger<ComfortnetCommandData, Comfortnet *> {
 public:
  explicit ComfortnetCommandTrigger(Comfortnet *parent, uint16_t control_command, uint8_t target_device_type);
};

class ComfortnetPacketTrigger : public esphome::Trigger<ComfortnetPacketData, Comfortnet *> {
 public:
  explicit ComfortnetPacketTrigger(Comfortnet *parent, uint8_t packet_type, uint8_t target_device_type,
                                   bool register_polling, bool poll_once);
};

}  // namespace comfortnet
