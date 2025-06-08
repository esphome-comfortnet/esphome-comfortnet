#include "esphome/core/log.h"

#include "automation.h"

static const char *const TAG = "comfortnet.automation";

namespace comfortnet {

ComfortnetCommandTrigger::ComfortnetCommandTrigger(Comfortnet *parent, uint16_t control_command,
                                                   uint8_t target_device_type) {
  parent->register_command_listener(static_cast<CommandType>(control_command),
                                    [this, target_device_type, parent](const ComfortnetCommandData &data) {
                                      if (data.node_type == static_cast<NodeType>(target_device_type) ||
                                          static_cast<NodeType>(target_device_type) == NodeType::ANY) {
                                        this->trigger(data, parent);
                                      }
                                    });
}

ComfortnetPacketTrigger::ComfortnetPacketTrigger(Comfortnet *parent, uint8_t packet_type, uint8_t target_device_type,
                                                 bool register_polling, bool poll_once) {
  parent->register_packet_listener(static_cast<MessageType>(packet_type),
                                   [this, target_device_type, parent](const ComfortnetPacketData &data) {
                                     if (data.node_type == static_cast<NodeType>(target_device_type) ||
                                         static_cast<NodeType>(target_device_type) == NodeType::ANY) {
                                       this->trigger(data, parent);
                                     }
                                   });
  if (register_polling) {
    if (static_cast<NodeType>(target_device_type) == NodeType::ANY) {
      ESP_LOGE(TAG, "Cannot register polling with ANY target type!");
      return;
    }
    if (packet_type == 0) {
      ESP_LOGE(TAG, "Cannot register polling with R2R packet type!");
      return;
    }
    parent->register_device_polling(static_cast<NodeType>(target_device_type),
                                    PACKET_REQUEST(static_cast<MessageType>(packet_type)), poll_once);
  }
}

}  // namespace comfortnet
