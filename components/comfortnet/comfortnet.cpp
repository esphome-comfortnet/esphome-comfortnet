#include "comfortnet.h"
#ifndef ARDUINO
#include "esp_timer.h"
#endif

namespace comfortnet {

static const char *const TAG = "comfortnet";

static const uint32_t RECEIVE_TIMEOUT = 500;   // How long we will wait to receive a full message
static const uint32_t REQUEST_TIMEOUT = 3000;  // How long we will wait for a reply to a request
static const uint32_t NETWORK_TIMEOUT =
    120000;  // We need to make sure our address is renewed at least every 120 seconds (Networking Specification 9.1.2)
static const uint32_t SILENCE_TIMEOUT = 60000;  // How long we will wait for any message, before assuming the network is
                                                // unavailable (This is not official spec,
                                                // but for monitoring purposes. May need adjustment.)

// Packet information
static const uint8_t PACKET_HEADER_SIZE = 10;
static const uint8_t PACKET_CRC_SIZE = 2;

// Header indices (Relative to packet)
static const uint8_t DESTINATION_ADDRESS_POS = 0;
static const uint8_t SOURCE_ADDRESS_POS = 1;
static const uint8_t SUBNET_POS = 2;
static const uint8_t SEND_METHOD_POS = 3;
static const uint8_t SEND_PARAMETER_1_POS = 4; /* Extra data for send method */
static const uint8_t SEND_PARAMETER_2_POS = 5; /* Generally 0 on sending, as coordinator will fill it with the ID of the
                                    target device when routing packets */
static const uint8_t SOURCE_NODE_TYPE_POS = 6;
static const uint8_t MESSAGE_TYPE_POS = 7;
static const uint8_t PACKET_NUMBER_POS =
    8; /* Defined in ClimateTalk Alliance CT2.0 CT-485 API Reference Revision 01 4.3 - Packet Number*/
static const uint8_t PAYLOAD_LENGTH_POS = 9; /* 0-MAX_PAYLOAD_SIZE */

static const uint8_t CMD_HEADER_SIZE = 2;

// Data indices (Relative to data, not packet)
static const uint8_t ACK_POS = 0;

static const uint8_t CONTROL_CMD_POS = 0;
static const uint8_t CONTROL_CMD_SIZE = 2;  // Length of control command header

static const uint8_t DISCOVERY_NODE_TYPE_POS = 0;

static const uint8_t TOKEN_OFFER_NODE_TYPE_POS = 0;

static const uint8_t ADDRESS_NODE_ID_POS = 0;
static const uint8_t ADDRESS_SUBNET_POS = 1;
static const uint8_t ADDRESS_MAC_POS = 2;

// ComfortnetData keys for listeners
static const std::string DATA_KEY_NETWORK_STATUS = "NETWORK_STATUS";

void Comfortnet::setup() {
  if (flow_control_pin_ != nullptr) {
    flow_control_pin_->setup();
  }
  mac_address_.setRandom();
}

void Comfortnet::dump_config() {
  ESP_LOGCONFIG(TAG, "ComfortNet:");
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
  ESP_LOGCONFIG(TAG, "  MAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", mac_address_.mac[0], mac_address_.mac[1],
                mac_address_.mac[2], mac_address_.mac[3], mac_address_.mac[4], mac_address_.mac[5], mac_address_.mac[6],
                mac_address_.mac[7]);
  ESP_LOGCONFIG(TAG, "  Device Type: %02x", device_type_);
}

uint16_t Comfortnet::calculate_crc_(const uint8_t *data, uint8_t data_len) {
  uint8_t sum1 = 0xAA;  // Fletcher seed
  uint8_t sum2 = 0;

  for (short i = 0; i < data_len; i++) {
    sum1 = (sum1 + data[i]) % 0xFF;
    sum2 = (sum2 + sum1) % 0xFF;
  }

  uint8_t tmp = 0xFF - ((sum1 + sum2) % 0xFF);
  return (static_cast<uint16_t>(tmp) << 8) | static_cast<uint16_t>(0xFF - ((sum1 + tmp) % 0xFF));
}

/**
 * Defined in ClimateTalk Alliance CT2.0 CT-485 Networking Specification Revision 01
 * 11.1 Slot Delay
 */
#define MINIMUM_SLOT_DELAY \
  100  // Additionally, bus must be silent for at least 100ms before we can speak (Networking Specification 9.5)
#define MAXIMUM_SLOT_DELAY 2500

uint32_t Comfortnet::generate_slot_delay_() {
#ifdef ARDUINO
  return random(MINIMUM_SLOT_DELAY, MAXIMUM_SLOT_DELAY);
#else
  return (esp_random() % (MAXIMUM_SLOT_DELAY - MINIMUM_SLOT_DELAY + 1)) + MINIMUM_SLOT_DELAY;
#endif
}

void Comfortnet::set_node_list_(const uint8_t *data, uint8_t data_len) {
  for (uint8_t i = 0; i < data_len; i++) {
    node_list_[i] = static_cast<NodeType>(data[i]);
  }
  node_list_size_ = data_len;
}

void Comfortnet::handle_message_(bool is_tx, uint32_t now) {
  const uint8_t *data = is_tx ? tx_message_.data() : rx_message_.data();
  uint8_t length = is_tx ? tx_message_.size() : rx_message_.size();

  // ESP_LOGD(TAG, "[RAW DUMP] %s", format_hex_pretty(data, packet->payload_length_ + PACKET_HEADER_SIZE +
  // PACKET_CRC_SIZE).c_str());

  NodeAddress dst_adr = static_cast<NodeAddress>(data[DESTINATION_ADDRESS_POS]);
  NodeAddress src_adr = static_cast<NodeAddress>(data[SOURCE_ADDRESS_POS]);
  Subnet subnet = static_cast<Subnet>(data[SUBNET_POS]);
  SendMethod send_method = static_cast<SendMethod>(data[SEND_METHOD_POS]);
  uint8_t send_param_1 = data[SEND_PARAMETER_1_POS];
  uint8_t send_param_2 = data[SEND_PARAMETER_2_POS];
  NodeType source_node_type = static_cast<NodeType>(data[SOURCE_NODE_TYPE_POS]);
  MessageType message_type = static_cast<MessageType>(data[MESSAGE_TYPE_POS]);
  uint8_t packet_number = data[PACKET_NUMBER_POS];
  uint8_t payload_len = data[PAYLOAD_LENGTH_POS];
  // Packet data with len=payload_len
  const uint8_t *payload = data + PACKET_HEADER_SIZE;

  // Checksum validation by reading last 2 bytes after the data
  uint16_t crc = (data[PACKET_HEADER_SIZE + payload_len] << 8) | data[PACKET_HEADER_SIZE + payload_len + 1];
  uint16_t crc_check = calculate_crc_(data, PACKET_HEADER_SIZE + payload_len);
  if (crc != crc_check) {
    ESP_LOGW(TAG, "Checksum mismatch. Expected 0x%04X, got 0x%04X", crc, crc_check);
    return;
  }

  // Notice, the checksum and payload are printed out of order here for viewing convenience!
  ESP_LOGD(TAG,
           "Dir | Dest | Src  | Subnet | Meth | Params | SrcNode | MsgType | PktNum | Len | Checksum | Payload HEX");
  ESP_LOGD(
      TAG,
      "%s  | 0x%02X | 0x%02X | 0x%02X   | 0x%02X | 0x%04X | 0x%02X    | 0x%02X    | 0x%02X   | %-3u | 0x%04X   | %s",
      is_tx ? "TX" : "RX", dst_adr, src_adr, subnet, send_method, (send_param_1 << 8) | send_param_2, source_node_type,
      message_type, packet_number, payload_len, crc, esphome::format_hex_pretty(payload, payload_len).c_str());
  if (is_tx) {
    if (message_type == MessageType::TOKEN_OFFER_RESPONSE) {
      // Most likely we won the token offer broadcast
      // We should only respond to 1 token offer per dataflow cycle to give other devices a chance
      has_won_token_broadcast_ = true;
    }
    // Stop here if this is a transmitted message
    return;
  }

  bool is_broadcast = dst_adr == NodeAddress::BROADCAST && (subnet == this->subnet_ || subnet == Subnet::BROADCAST);

  // Signals the start of a new dataflow cycle
  if (message_type == MessageType::NODE_DISCOVERY) {
    has_won_token_broadcast_ = false;
  }

  std::vector<uint8_t> return_payload;
  // Network member logic
  if (node_id_ != static_cast<NodeAddress>(0)) {
    /**
     * We are a network member
     */
    if (is_broadcast) {
      /**
       * Message is broadcast to us
       */
      if (message_type == MessageType::ADDRESS_CONFIRMATION) {
        uint8_t idnum = static_cast<uint8_t>(node_id_);
        this->last_address_confirm_time_ = now;
        if (idnum >= payload_len || static_cast<NodeType>(payload[idnum]) != device_type_) {
          ESP_LOGW(TAG, "Not in node list, disconnecting");
          disconnect_();
        }
      } else if (message_type == MessageType::SET_ADDRESS) {
        for (uint8_t i = 0; i < MAC_ADDRESS_SIZE; i++) {
          if (payload[ADDRESS_MAC_POS + i] != this->mac_address_.mac[i]) {
            return;  // Not our MAC
          }
        }
        for (uint8_t i = 0; i < SESSION_ID_SIZE; i++) {
          if (payload[ADDRESS_MAC_POS + MAC_ADDRESS_SIZE + i] != this->session_id_.sessionid[i]) {
            return;  // Not our session ID
          }
        }
        if (payload[ADDRESS_MAC_POS + MAC_ADDRESS_SIZE + SESSION_ID_SIZE] != 0x01) {
          ESP_LOGW(TAG, "Failed to get address!");  // Write byte must be 0x01
          return;
        }
        NodeAddress start_id = this->node_id_;
        this->last_address_confirm_time_ = now;
        this->node_id_ = static_cast<NodeAddress>(payload[ADDRESS_NODE_ID_POS]);
        this->subnet_ = static_cast<Subnet>(payload[ADDRESS_SUBNET_POS]);
        return_payload.push_back(static_cast<uint8_t>(this->node_id_));
        return_payload.push_back(static_cast<uint8_t>(this->subnet_));
        this->mac_address_.write(return_payload);
        this->session_id_.write(return_payload);
        return_payload.push_back(0x01);  // Write byte must be 0x01
        transmit_message_(NodeAddress::COORDINATOR, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0,
                          this->device_type_, PACKET_RESPONSE(message_type),
                          PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload);
        this->awaiting_discovery_ = false;
        ESP_LOGI(TAG, "Network address reassigned: 0x%02X (Old: 0x%02X)", this->node_id_, start_id);
      } else if (message_type == MessageType::TOKEN_OFFER && !has_won_token_broadcast_ &&
                 (pending_messages_.size() > 0 || polling_queue_.size() > 0)) {
        NodeType offer_node_type = static_cast<NodeType>(payload[TOKEN_OFFER_NODE_TYPE_POS]);
        if (offer_node_type == NodeType::ANY || offer_node_type == this->device_type_) {
          return_payload.push_back(static_cast<uint8_t>(this->node_id_));
          return_payload.push_back(static_cast<uint8_t>(this->subnet_));
          this->mac_address_.write(return_payload);
          this->session_id_.write(return_payload);
          transmit_message_(NodeAddress::COORDINATOR, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0,
                            this->device_type_, PACKET_RESPONSE(message_type),
                            PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload, true);
        }
      }
    } else if (dst_adr == this->node_id_ && subnet == this->subnet_) {
      /**
       * Message is addressed to us
       */
      if (message_type == MessageType::SET_NETWORK_NODE_LIST) {
        /**
         * Core network packet
         */
        set_node_list_(payload, payload_len);
        transmit_message_(src_adr, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0, this->device_type_,
                          MessageType::SET_NETWORK_NODE_LIST_RESPONSE,
                          PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1),
                          reinterpret_cast<uint8_t *>(node_list_ + 0), node_list_size_);
      } else if (message_type == MessageType::REQUEST_TO_RECEIVE_RESPONSE) {
        /**
         * R2R section
         */
        if (pending_messages_.size() == 0 && polling_queue_.size() > 0) {
          // If we have no commands to send, queue up a request to poll a device's status
          PollQueueEntry dev = polling_queue_.front();
          std::vector<uint8_t> payload;
          bool can_reply = true;
          if (dev.poll_message == MessageType::GET_STATUS || dev.poll_message == MessageType::GET_SENSOR_DATA ||
              dev.poll_message == MessageType::GET_IDENTIFICATION ||
              dev.poll_message == MessageType::GET_CONFIGURATION) {
            // Empty payload
          } else {
            ESP_LOGW(TAG, "Unable to handle poll request for message type 0x%02X to node type 0x%02X", dev.poll_message,
                     dev.node_type);
            polling_queue_.erase(polling_queue_.begin());
            can_reply = false;
          }
          if (can_reply) {
            pending_messages_.push((struct PendingMessageToType) {
                dev.node_type,
                dev.poll_message,
                payload,
            });
          }
        }
        if (!this->r2r_reply_.empty()) {
          /**
           * We previously received a packet that this R2R is confirming
           */
          tx_message_.clear();
          tx_message_.insert(tx_message_.end(), r2r_reply_.begin(), r2r_reply_.end());
          r2r_reply_.clear();
          message_queued_ = QueuedMessageType::NORMAL;
        } else if (pending_messages_.size() > 0) {
          /**
           * We have packets we need to send, send them!
           */
          PendingMessage msg = pending_messages_.front();
          return_payload.insert(return_payload.end(), msg.payload.begin(), msg.payload.end());
          transmit_message_(src_adr, this->node_id_, this->subnet_, msg.send_method, msg.send_param_1, 0,
                            this->device_type_, msg.packet_type,
                            PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload);
        } else {
          /**
           * We have nothing to send, just ACK
           */
          return_payload.push_back(R2R_ACK);
          this->mac_address_.write(return_payload);
          this->session_id_.write(return_payload);
          transmit_message_(src_adr, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0, this->device_type_,
                            MessageType::REQUEST_TO_RECEIVE_RESPONSE,
                            PACKET_NUMBER(true, this->subnet_ == Subnet::VERSION_1), return_payload);
        }
      } else {
        /**
         * All other packets addressed to us.
         * We should filter out packets we can't respond to and send NAKs or something instead of always sending an
         * ACK...
         */
        MessageAckAction should_ack = MessageAckAction::UNKNOWN;
        if (pending_messages_.size() > 0) {
          // Check if this is a reply to our request
          if (message_type == pending_messages_.front().packet_type &&
              send_param_1 == pending_messages_.front().send_param_1) {
            should_ack = MessageAckAction::NONE;
            if (payload_len < 1 || payload[ACK_POS] != R2R_ACK) {
              ESP_LOGW(TAG, "Corodinator did not ACK our 0x%02X", message_type);
            }
          } else if (message_type == PACKET_RESPONSE(pending_messages_.front().packet_type) &&
                     send_param_1 == pending_messages_.front().send_param_1) {
            should_ack = MessageAckAction::ACK;
            pending_messages_.pop();
          }
        }
        if (should_ack == MessageAckAction::UNKNOWN && message_type == MessageType::GET_NODE_ID) {
          should_ack = MessageAckAction::ACK;
          return_payload.push_back(static_cast<uint8_t>(this->device_type_));
          this->mac_address_.write(return_payload);
          this->session_id_.write(return_payload);
          write_message_to_buffer_(r2r_reply_, src_adr, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0,
                                   this->device_type_, PACKET_RESPONSE(message_type),
                                   PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload.data(),
                                   return_payload.size(), false, false);
        } else if (should_ack == MessageAckAction::UNKNOWN && message_type == MessageType::GET_NODE_ID_RESPONSE) {
          should_ack = MessageAckAction::ACK;
        } else if (should_ack == MessageAckAction::UNKNOWN &&
                   message_type == MessageType::NETWORK_SHARED_DATA_SECTOR_IMAGE_READ_WRITE_REQUEST) {
          /**
           * For redundant data storage across all network nodes
           */
          NodeType requesting_type = static_cast<NodeType>(payload[0] & 0x7F);  // Clear bit 7 and extract the node type
          std::vector<uint8_t> *shared_data = nullptr;
          auto iter = network_shared_data_.find(requesting_type);
          if (iter == network_shared_data_.end()) {
            shared_data = &network_shared_data_[requesting_type];
          } else {
            shared_data = &iter->second;
          }
          should_ack = MessageAckAction::ACK;
          return_payload.push_back(static_cast<uint8_t>(requesting_type));
          if ((payload[0] & 0x80) == 0) {  // Write operation
            shared_data->clear();
            shared_data->insert(shared_data->end(), payload + 1, payload + payload_len);
          }
          return_payload.insert(return_payload.end(), shared_data->begin(), shared_data->end());
          write_message_to_buffer_(r2r_reply_, src_adr, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0,
                                   this->device_type_, PACKET_RESPONSE(message_type),
                                   PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload.data(),
                                   return_payload.size(), false, false);
        } else if (should_ack == MessageAckAction::UNKNOWN &&
                   message_type == MessageType::NETWORK_SHARED_DATA_SECTOR_IMAGE_READ_WRITE_REQUEST_RESPONSE) {
          should_ack = MessageAckAction::NONE;
          if (payload[ACK_POS] != R2R_ACK) {
            ESP_LOGW(TAG, "Received error during network shared data response!");
          }
        }
        if (should_ack == MessageAckAction::ACK) {
          return_payload.clear();
          return_payload.push_back(R2R_ACK);
          this->mac_address_.write(return_payload);
          this->session_id_.write(return_payload);
          transmit_message_(src_adr, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0, this->device_type_,
                            message_type, PACKET_NUMBER(true, this->subnet_ == Subnet::VERSION_1), return_payload);
        } else if (should_ack == MessageAckAction::NAK) {
          ESP_LOGW(TAG, "We are supposed to NAK to 0x%02X, but don't know how!", message_type);
        } else if (!PACKET_IS_DATAFLOW(packet_number) && should_ack == MessageAckAction::UNKNOWN) {
          ESP_LOGW(TAG, "We are supposed to respond to 0x%02X, but don't know how!", message_type);
        }
      }
    }
  } else {
    /**
     * We are not yet a network member
     */
    if (!awaiting_discovery_ && message_type == MessageType::NODE_DISCOVERY) {
      NodeType discovery_node_type = static_cast<NodeType>(payload[DISCOVERY_NODE_TYPE_POS]);
      if (discovery_node_type == NodeType::ANY || discovery_node_type == this->device_type_) {
        ESP_LOGI(TAG, "Received discovery request, responding...");
        session_id_.setRandom();  // Generate a session ID
        return_payload.push_back(static_cast<uint8_t>(this->device_type_));
        return_payload.push_back(0x00);  // Reserved
        this->mac_address_.write(return_payload);
        this->session_id_.write(return_payload);
        transmit_message_(NodeAddress::COORDINATOR, this->node_id_, Subnet::BROADCAST, SendMethod::NO_ROUTE, 0, 0,
                          this->device_type_, PACKET_RESPONSE(message_type),
                          PACKET_NUMBER(false, this->ct_version_ == 1), return_payload, true);
        awaiting_discovery_ = true;
      }
    } else if (is_broadcast && message_type == MessageType::SET_ADDRESS) {
      for (uint8_t i = 0; i < MAC_ADDRESS_SIZE; i++) {
        if (payload[ADDRESS_MAC_POS + i] != this->mac_address_.mac[i]) {
          return;  // Not our MAC
        }
      }
      for (uint8_t i = 0; i < SESSION_ID_SIZE; i++) {
        if (payload[ADDRESS_MAC_POS + MAC_ADDRESS_SIZE + i] != this->session_id_.sessionid[i]) {
          return;  // Not our session ID
        }
      }
      if (payload[ADDRESS_MAC_POS + MAC_ADDRESS_SIZE + SESSION_ID_SIZE] != 0x01) {
        ESP_LOGW(TAG, "Failed to get address!");  // Write byte must be 0x01
        return;
      }
      NodeAddress start_id = this->node_id_;
      this->last_address_confirm_time_ = now;
      this->node_id_ = static_cast<NodeAddress>(payload[ADDRESS_NODE_ID_POS]);
      this->subnet_ = static_cast<Subnet>(payload[ADDRESS_SUBNET_POS]);
      return_payload.push_back(static_cast<uint8_t>(this->node_id_));
      return_payload.push_back(static_cast<uint8_t>(this->subnet_));
      this->mac_address_.write(return_payload);
      this->session_id_.write(return_payload);
      return_payload.push_back(0x01);  // Write byte must be 0x01
      transmit_message_(NodeAddress::COORDINATOR, this->node_id_, this->subnet_, SendMethod::NO_ROUTE, 0, 0,
                        this->device_type_, PACKET_RESPONSE(message_type),
                        PACKET_NUMBER(false, this->subnet_ == Subnet::VERSION_1), return_payload);
      this->awaiting_discovery_ = false;
      if (start_id == static_cast<NodeAddress>(0)) {
        ESP_LOGI(TAG, "Joined network as address: 0x%02X", this->node_id_);
        call_listener_(DATA_KEY_NETWORK_STATUS,
                       (struct ComfortnetData) {this->device_type_, ComfortnetData::DataType::BOOLEAN, true});
      } else if (start_id != this->node_id_) {
        ESP_LOGI(TAG, "Network address reassigned: 0x%02X (Old: 0x%02X)", this->node_id_, start_id);
      }
    }
  }
  // End network member logic

  // Network eavesdropping logic
  if (PACKET_IS_DATAFLOW(packet_number) && payload_len == 17 && payload[ACK_POS] == R2R_ACK &&
      src_adr != NodeAddress::BROADCAST &&
      static_cast<uint8_t>(src_adr) < MAX_PAYLOAD_SIZE) {  // Simple check for ACK messages
    for (uint8_t i = 0; i < MAC_ADDRESS_SIZE; i++) {
      this->node_mac_list_[static_cast<uint8_t>(src_adr)].mac[i] = payload[i + 1];  // Copy MAC to list
    }
  }
  if (message_type == MessageType::SET_CONTROL_COMMAND || message_type == MessageType::SET_CONTROL_COMMAND_RESPONSE) {
    if (message_type == MessageType::SET_CONTROL_COMMAND_RESPONSE && payload_len <= CONTROL_CMD_SIZE) {
      return;  // This is just an ACK, so we can't get any useful data from it
    }
    CommandType command_type = static_cast<CommandType>((payload[CONTROL_CMD_POS + 1] << 8) | payload[CONTROL_CMD_POS]);
    const uint8_t *cmd_payload = payload + CONTROL_CMD_SIZE;
    uint8_t cmd_payload_len = payload_len - CONTROL_CMD_SIZE;
    ESP_LOGD(TAG, "Command | Payload HEX");
    ESP_LOGD(TAG, "0x%04X  | %s", command_type, esphome::format_hex_pretty(cmd_payload, cmd_payload_len).c_str());
    call_command_listener_((struct ComfortnetCommandData) {
        message_type == MessageType::SET_CONTROL_COMMAND ? get_node_type_(dst_adr) : source_node_type,
        get_node_mac_(message_type == MessageType::SET_CONTROL_COMMAND ? dst_adr : src_adr), command_type,
        message_type == MessageType::SET_CONTROL_COMMAND_RESPONSE, cmd_payload, cmd_payload_len});
  } else if (!PACKET_IS_DATAFLOW(packet_number) && (message_type == MessageType::GET_STATUS_RESPONSE ||
                                                    message_type == MessageType::GET_SENSOR_DATA_RESPONSE ||
                                                    message_type == MessageType::GET_CONFIGURATION_RESPONSE ||
                                                    message_type == MessageType::GET_IDENTIFICATION_RESPONSE)) {
    call_packet_listener_(
        (struct ComfortnetPacketData) {source_node_type, get_node_mac_(src_adr), message_type, payload, payload_len});
  }
  // End network eavesdropping logic
}

void Comfortnet::write_message_to_buffer_(std::vector<uint8_t> &buffer, NodeAddress dst_adr, NodeAddress src_adr,
                                          Subnet subnet, SendMethod send_method, uint8_t send_param_1,
                                          uint8_t send_param_2, NodeType src_node_type, MessageType msg_type,
                                          uint8_t packet_num, const uint8_t *data, uint8_t data_len, bool queue_send,
                                          bool require_arbitration) {
  buffer.clear();
  // Write packet header
  buffer.push_back(static_cast<uint8_t>(dst_adr));
  buffer.push_back(static_cast<uint8_t>(src_adr));
  buffer.push_back(static_cast<uint8_t>(subnet));
  buffer.push_back(static_cast<uint8_t>(send_method));
  buffer.push_back(send_param_1);
  buffer.push_back(send_param_2);
  buffer.push_back(static_cast<uint8_t>(src_node_type));
  buffer.push_back(static_cast<uint8_t>(msg_type));
  buffer.push_back(packet_num);
  buffer.push_back(data_len);
  // Write packet
  buffer.insert(buffer.end(), data, data + data_len);
  // Calculate CRC
  uint16_t crc = calculate_crc_(buffer.data(), PACKET_HEADER_SIZE + data_len);
  buffer.push_back((crc >> 8) & 0xFF);
  buffer.push_back(crc & 0xFF);

  if (queue_send) {
    message_queued_ = require_arbitration ? QueuedMessageType::ARBITRATION : QueuedMessageType::NORMAL;
    if (message_queued_ == QueuedMessageType::ARBITRATION) {
      slot_delay_ = generate_slot_delay_();
      ESP_LOGI(TAG, "Will arbitrate with slot delay of %u", slot_delay_);
    }
  }
}

void Comfortnet::read_buffer_(int bytes_available, uint32_t now) {
  uint8_t bytes[bytes_available];

  if (!this->read_array(bytes, bytes_available)) {
    return;
  }

  for (int i = 0; i < bytes_available; i++) {
    rx_message_.push_back(bytes[i]);

    if (rx_message_.size() > PAYLOAD_LENGTH_POS &&
        rx_message_.size() == PACKET_HEADER_SIZE + rx_message_[PAYLOAD_LENGTH_POS] + PACKET_CRC_SIZE) {
      // We have a full message
      this->handle_message_(false, now);
      rx_message_.clear();
    }
  }
}

void Comfortnet::loop() {
#ifdef ARDUINO
  const uint32_t now = millis();
#else
  const uint32_t now = (uint32_t) (esp_timer_get_time() / 1000);
#endif
  if (message_queued_ != QueuedMessageType::NONE) {
    uint32_t delay = message_queued_ == QueuedMessageType::ARBITRATION ? this->slot_delay_ : MINIMUM_SLOT_DELAY;
    if (now - this->last_read_time_ > delay) {
      if (this->available() > 0) {
        // Final check if line is busy
        message_queued_ = QueuedMessageType::NONE;
        slot_delay_ = 0;
        tx_message_.clear();
        if (awaiting_discovery_) {
          session_id_.clear();
        }
        awaiting_discovery_ = false;
      } else {
        if (this->flow_control_pin_ != nullptr) {
          this->flow_control_pin_->digital_write(true);
        }
        this->write_array(tx_message_.data(), tx_message_.size());
        this->flush();
        if (this->flow_control_pin_ != nullptr) {
          this->flow_control_pin_->digital_write(false);
        }
        this->handle_message_(true, now);
        message_queued_ = QueuedMessageType::NONE;
        slot_delay_ = 0;
        tx_message_.clear();
      }
    }
  }

  // Read Everything that is in the buffer
  int bytes_available = this->available();
  if (bytes_available > 0) {
    this->last_read_time_ = now;
    this->read_buffer_(bytes_available, now);
  } else if (node_id_ != static_cast<NodeAddress>(0) && now - this->last_read_time_ > SILENCE_TIMEOUT) {
    ESP_LOGW(TAG, "Network appears to be offline");
    disconnect_();
    // Disconnect
  } else if ((now - this->last_read_time_ > RECEIVE_TIMEOUT) && !rx_message_.empty()) {
    ESP_LOGW(TAG, "Timed out reading partial message");
    rx_message_.clear();
  }
  if (node_id_ != static_cast<NodeAddress>(0) && (now - this->last_address_confirm_time_ > NETWORK_TIMEOUT)) {
    ESP_LOGW(TAG, "Dropped from network, discarding session information");
    disconnect_();
  }
}

void Comfortnet::disconnect_() {
  rx_message_.clear();
  tx_message_.clear();
  r2r_reply_.clear();
  message_queued_ = QueuedMessageType::NONE;
  slot_delay_ = 0;
  awaiting_discovery_ = false;
  has_won_token_broadcast_ = false;

  node_id_ = static_cast<NodeAddress>(0);
  subnet_ = Subnet::BROADCAST;
  session_id_.clear();
  call_listener_(DATA_KEY_NETWORK_STATUS,
                 (struct ComfortnetData) {this->device_type_, ComfortnetData::DataType::BOOLEAN, false});
}

}  // namespace comfortnet
