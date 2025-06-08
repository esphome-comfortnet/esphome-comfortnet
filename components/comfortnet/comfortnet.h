#pragma once

#include <set>
#include <map>
#include <queue>
#include <variant>
#include <optional>
#include <algorithm>
#include "types.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/uart/uart.h"

namespace comfortnet {

enum class QueuedMessageType : uint8_t {
  NONE = 0,
  NORMAL = 1,
  ARBITRATION = 2,
};

enum class MessageAckAction : uint8_t {
  NONE = 0,
  ACK = 1,
  NAK = 2,
  UNKNOWN = 3,
};

struct PollQueueEntry {
  NodeType node_type;
  MessageType poll_message;
  bool poll_once;

  PollQueueEntry(NodeType node_type, MessageType poll_message, bool poll_once)
      : node_type(node_type), poll_message(poll_message), poll_once(poll_once){};

  bool operator==(const PollQueueEntry &other) const {
    return node_type == other.node_type && poll_message == other.poll_message;
  }
};

struct PendingMessage {
  SendMethod send_method;
  uint8_t send_param_1;
  MessageType packet_type;
  std::vector<uint8_t> payload;

  PendingMessage(SendMethod send_method, uint8_t send_param_1, MessageType packet_type, std::vector<uint8_t> payload)
      : send_method(send_method), send_param_1(send_param_1), packet_type(packet_type), payload(payload){};
};

struct PendingMessageByCommand : PendingMessage {
  SendMethodControlCommand command_type;

  PendingMessageByCommand(SendMethodControlCommand command_type, MessageType packet_type, std::vector<uint8_t> payload)
      : command_type(command_type),
        PendingMessage(SendMethod::CONTROL_COMMAND, static_cast<uint8_t>(command_type), packet_type, payload){};
};

struct PendingMessageToType : PendingMessage {
  NodeType node_type;

  PendingMessageToType(NodeType node_type, MessageType packet_type, std::vector<uint8_t> payload)
      : node_type(node_type),
        PendingMessage(SendMethod::NODE_TYPE, static_cast<uint8_t>(node_type), packet_type, payload){};
};

struct PendingMessageToAddress : PendingMessage {
  NodeAddress dest_address;

  PendingMessageToAddress(NodeAddress dest_address, MessageType packet_type, std::vector<uint8_t> payload)
      : dest_address(dest_address),
        PendingMessage(SendMethod::NODE_ID, static_cast<uint8_t>(dest_address), packet_type, payload){};
};

struct ComfortnetData {
  NodeType device_type;
  enum class DataType { BOOLEAN, FLOAT, STRING } type;
  using DataVariant = std::variant<bool, float, std::string>;
  DataVariant data;

  ComfortnetData(NodeType device_type, DataType type, DataVariant data)
      : device_type(device_type), type(type), data(data){};
};

struct ComfortnetCommandData {
  NodeType node_type;
  std::optional<MacAddress> node_mac;
  CommandType cmd_type;
  bool response;
  const uint8_t *payload;
  uint8_t payload_len;

  ComfortnetCommandData(NodeType node_type, std::optional<MacAddress> node_mac, CommandType cmd_type, bool response,
                        const uint8_t *payload, uint8_t payload_len)
      : node_type(node_type),
        node_mac(node_mac),
        cmd_type(cmd_type),
        response(response),
        payload(payload),
        payload_len(payload_len){};
};

struct ComfortnetPacketData {
  NodeType node_type;
  std::optional<MacAddress> node_mac;
  MessageType packet_type;
  MessageType packet_type_request;
  MessageType packet_type_response;
  const uint8_t *payload;
  uint8_t payload_len;

  ComfortnetPacketData(NodeType node_type, std::optional<MacAddress> node_mac, MessageType packet_type,
                       const uint8_t *payload, uint8_t payload_len)
      : node_type(node_type),
        node_mac(node_mac),
        packet_type(packet_type),
        packet_type_request(PACKET_REQUEST(packet_type)),
        packet_type_response(PACKET_RESPONSE(packet_type)),
        payload(payload),
        payload_len(payload_len){};
};

struct DBIDDatagram {
  uint8_t dbid_tag;
  uint8_t db_len;
  const uint8_t *data;

  DBIDDatagram(uint8_t dbid_tag, uint8_t db_len, const uint8_t *data)
      : dbid_tag(dbid_tag), db_len(db_len), data(data){};
};

class Comfortnet : public esphome::Component, public esphome::uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_device_type(uint8_t type) { device_type_ = static_cast<NodeType>(type); }
  void set_flow_control_pin(esphome::GPIOPin *flow_control_pin) { this->flow_control_pin_ = flow_control_pin; }

  void set_update_interval(uint32_t interval_millis) { update_interval_millis_ = interval_millis; }

  inline void register_listener(std::string sensor_key, std::function<void(ComfortnetData)> callback) {
    std::vector<std::function<void(ComfortnetData)>> *listener_vector = nullptr;
    auto iter = this->listeners_.find(sensor_key);
    if (iter == this->listeners_.end()) {
      listener_vector = &this->listeners_[sensor_key];
    } else {
      listener_vector = &iter->second;
    }
    listener_vector->push_back(callback);
  };
  inline void register_command_listener(CommandType command_type, std::function<void(ComfortnetCommandData)> callback) {
    std::vector<std::function<void(ComfortnetCommandData)>> *listener_vector = nullptr;
    auto iter = this->command_listeners_.find(command_type);
    if (iter == this->command_listeners_.end()) {
      listener_vector = &this->command_listeners_[command_type];
    } else {
      listener_vector = &iter->second;
    }
    listener_vector->push_back(callback);
  };
  inline void register_packet_listener(MessageType message_type, std::function<void(ComfortnetPacketData)> callback) {
    std::vector<std::function<void(ComfortnetPacketData)>> *listener_vector = nullptr;
    auto iter = this->packet_listeners_.find(message_type);
    if (iter == this->packet_listeners_.end()) {
      listener_vector = &this->packet_listeners_[message_type];
    } else {
      listener_vector = &iter->second;
    }
    listener_vector->push_back(callback);
  };

  inline void register_device_polling(NodeType node_type, MessageType poll_message, bool poll_once) {
    auto it = std::find(polling_queue_.begin(), polling_queue_.end(),
                        (struct PollQueueEntry){node_type, poll_message, poll_once});
    if (it == polling_queue_.end()) {
      polling_queue_.emplace_back(node_type, poll_message, poll_once);
    } else if (it.base()->poll_once && !poll_once) {
      // If the existing request says to poll once, but this one says to repeatedly pull, reconfigure the request.
      it.base()->poll_once = poll_once;
    }
  };
  /**
   * Moves the given device to the end of the poll priority list
   */
  inline void device_poll_to_end(NodeType node_type, MessageType poll_message) {
    auto it = std::find(polling_queue_.begin(), polling_queue_.end(),
                        (struct PollQueueEntry){node_type, poll_message, false});
    if (it != polling_queue_.end()) {
      if (it.base()->poll_once) {
        polling_queue_.erase(it);
      }
      std::rotate(it, it + 1, polling_queue_.end());
    }
  };

  inline void read_mdi(const uint8_t *data, uint8_t data_len, std::vector<DBIDDatagram> *parsed) {
    uint8_t i = 0;
    while (i < data_len) {
      if (i + 2 >= data_len) {
        return;
      }
      uint8_t tag = data[i++];
      uint8_t len = data[i++];
      if (i + len > data_len) {
        return;
      }
      parsed->emplace_back(tag, len, data + i);
      i += len;
    }
  };

  inline void queue_message(PendingMessage message) { pending_messages_.push(message); };

 protected:
  uint32_t update_interval_millis_{30000};
  void read_buffer_(int bytes_available, uint32_t now);
  void handle_message_(bool is_tx, uint32_t now);
  uint16_t calculate_crc_(const uint8_t *data, uint8_t data_len);
  uint32_t generate_slot_delay_();
  void set_node_list_(const uint8_t *data, uint8_t data_len);
  inline NodeType get_node_type_(NodeAddress address) {
    uint8_t addr = static_cast<uint8_t>(address);
    if (addr >= MAX_PAYLOAD_SIZE) {
      return NodeType::ANY;
    }
    return this->node_list_[addr];
  }
  inline std::optional<MacAddress> get_node_mac_(NodeAddress address) {
    uint8_t addr = static_cast<uint8_t>(address);
    if (addr >= MAX_PAYLOAD_SIZE) {
      return std::nullopt;
    }
    return this->node_mac_list_[addr];
  }
  void disconnect_();

  inline void call_listener_(std::string sensor_key, ComfortnetData data) {
    auto iter = this->listeners_.find(sensor_key);
    if (iter != this->listeners_.end()) {
      std::vector<std::function<void(ComfortnetData)>> listener_vector = iter->second;
      for (auto &callback : listener_vector) {
        callback(data);
      }
    }
  }
  inline void call_command_listener_(ComfortnetCommandData data) {
    auto iter = this->command_listeners_.find(data.cmd_type);
    if (iter != this->command_listeners_.end()) {
      std::vector<std::function<void(ComfortnetCommandData)>> listener_vector = iter->second;
      for (auto &callback : listener_vector) {
        callback(data);
      }
    }
  }
  inline void call_packet_listener_(ComfortnetPacketData data) {
    auto iter = this->packet_listeners_.find(data.packet_type);
    if (iter != this->packet_listeners_.end()) {
      std::vector<std::function<void(ComfortnetPacketData)>> listener_vector = iter->second;
      for (auto &callback : listener_vector) {
        callback(data);
      }
    }
  }

  inline void transmit_message_(NodeAddress dst_adr, NodeAddress src_adr, Subnet subnet, SendMethod send_method,
                                uint8_t send_param_1, uint8_t send_param_2, NodeType src_node_type,
                                MessageType msg_type, uint8_t packet_num, const std::vector<uint8_t> &data,
                                bool require_arbitration = false) {
    write_message_to_buffer_(tx_message_, dst_adr, src_adr, subnet, send_method, send_param_1, send_param_2,
                             src_node_type, msg_type, packet_num, data.data(), data.size(), true, require_arbitration);
  }
  inline void transmit_message_(NodeAddress dst_adr, NodeAddress src_adr, Subnet subnet, SendMethod send_method,
                                uint8_t send_param_1, uint8_t send_param_2, NodeType src_node_type,
                                MessageType msg_type, uint8_t packet_num, const uint8_t *data, uint8_t data_len,
                                bool require_arbitration = false) {
    write_message_to_buffer_(tx_message_, dst_adr, src_adr, subnet, send_method, send_param_1, send_param_2,
                             src_node_type, msg_type, packet_num, data, data_len, true, require_arbitration);
  }
  void write_message_to_buffer_(std::vector<uint8_t> &buffer, NodeAddress dst_adr, NodeAddress src_adr, Subnet subnet,
                                SendMethod send_method, uint8_t send_param_1, uint8_t send_param_2,
                                NodeType src_node_type, MessageType msg_type, uint8_t packet_num, const uint8_t *data,
                                uint8_t data_len, bool queue_send, bool require_arbitration);
  esphome::GPIOPin *flow_control_pin_{nullptr};

  std::vector<uint8_t> rx_message_;
  std::vector<uint8_t> tx_message_;
  std::vector<uint8_t> r2r_reply_;

  uint32_t last_read_time_{0};                                 // Last time any data was read
  uint32_t last_address_confirm_time_{0};                      // Last time our address was confirmed
  uint32_t slot_delay_{0};                                     // Calculated slot delay when we are arbitrating
  QueuedMessageType message_queued_{QueuedMessageType::NONE};  // Whether we should arbitrate, or are sending normally
  bool awaiting_discovery_;                                    // Whether we are in the discovery process
  bool has_won_token_broadcast_;                               // Devices can only win token offer once per dataflow

  MacAddress mac_address_;
  NodeType device_type_{NodeType::DIAGNOSTIC_DEVICE};
  NodeAddress node_id_{static_cast<NodeAddress>(0)};
  Subnet subnet_{Subnet::BROADCAST};
  SessionId session_id_;

  std::queue<PendingMessage> pending_messages_;
  std::vector<PollQueueEntry> polling_queue_;

  uint8_t node_list_size_ = 0;
  NodeType node_list_[MAX_PAYLOAD_SIZE];
  MacAddress node_mac_list_[MAX_PAYLOAD_SIZE];
  std::map<NodeType, std::vector<uint8_t>> network_shared_data_;

  std::map<std::string, std::vector<std::function<void(ComfortnetData)>>> listeners_;
  std::map<CommandType, std::vector<std::function<void(ComfortnetCommandData)>>> command_listeners_;
  std::map<MessageType, std::vector<std::function<void(ComfortnetPacketData)>>> packet_listeners_;
};

class ComfortnetClient {
 public:
  inline void set_comfortnet_parent(Comfortnet *parent) { this->parent_ = parent; };

 protected:
  Comfortnet *parent_;
};

}  // namespace comfortnet
