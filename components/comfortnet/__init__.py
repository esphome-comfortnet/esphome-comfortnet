import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, pins
from esphome.components import uart
from esphome.const import (
    CONF_FLOW_CONTROL_PIN,
    CONF_ID,
    CONF_TRIGGER_ID,
)
from esphome.cpp_helpers import gpio_pin_expression

DEPENDENCIES = ["uart"]

CONF_CT_VERSION = "ct_version"
CONF_DEVICE_TYPE = "device_type"
CONF_SENSOR_KEY = "data_key"
CONF_TARGET_DEVICE_TYPE = "target_device_type"
CONF_ON_CONTROL_COMMAND = "on_control_command"
CONF_CONTROL_COMMAND = "control_command"
CONF_ON_PACKET = "on_packet"
CONF_PACKET_TYPE = "packet_type"
CONF_REGISTER_PACKET_POLL = "register_polling"
CONF_PACKET_POLL_ONCE = "poll_once"

comfortnet_ns = cg.esphome_ns.namespace("comfortnet")
Comfortnet = comfortnet_ns.class_("Comfortnet", cg.Component, uart.UARTDevice)
ComfortnetPointer = comfortnet_ns.class_("Comfortnet*")
ComfortnetClient = comfortnet_ns.class_("ComfortnetClient")
ComfortnetCommandData = comfortnet_ns.class_("ComfortnetCommandData")
ComfortnetCommandTrigger = comfortnet_ns.class_(
    "ComfortnetCommandTrigger",
    automation.Trigger.template(ComfortnetCommandData, ComfortnetPointer),
)
ComfortnetPacketData = comfortnet_ns.class_("ComfortnetPacketData")
ComfortnetPacketTrigger = comfortnet_ns.class_(
    "ComfortnetPacketTrigger",
    automation.Trigger.template(ComfortnetPacketData, ComfortnetPointer),
)


def assign_declare_id(triggerClass, value):
    value = value.copy()
    value[CONF_TRIGGER_ID] = cv.declare_id(triggerClass)(value[CONF_TRIGGER_ID].id)
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Comfortnet),
            cv.Optional(CONF_CT_VERSION, default=2): cv.int_range(min=1, max=2),
            cv.Optional(CONF_DEVICE_TYPE, default=0x1E): cv.int_range(min=1, max=255),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_ON_CONTROL_COMMAND): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        ComfortnetCommandTrigger
                    ),
                    cv.Required(CONF_CONTROL_COMMAND): cv.uint16_t,
                    cv.Optional(CONF_TARGET_DEVICE_TYPE, default=0): cv.uint8_t,
                },
                extra_validators=lambda *args, **kwargs: assign_declare_id(
                    ComfortnetCommandTrigger, *args, **kwargs
                ),
            ),
            cv.Optional(CONF_ON_PACKET): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        ComfortnetPacketTrigger
                    ),
                    cv.Required(CONF_PACKET_TYPE): cv.uint8_t,
                    cv.Optional(CONF_TARGET_DEVICE_TYPE, default=0): cv.uint8_t,
                    cv.Optional(CONF_REGISTER_PACKET_POLL, default=False): cv.boolean,
                    cv.Optional(CONF_PACKET_POLL_ONCE, default=False): cv.boolean,
                },
                extra_validators=lambda *args, **kwargs: assign_declare_id(
                    ComfortnetPacketTrigger, *args, **kwargs
                ),
            ),
        }
    )
    .extend(cv.polling_component_schema("30s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

CONF_COMFORTNET_ID = "comfortnet_id"
COMFORTNET_CLIENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_COMFORTNET_ID): cv.use_id(Comfortnet),
        cv.Required(CONF_SENSOR_KEY): cv.string,
        cv.Optional(CONF_TARGET_DEVICE_TYPE, default=0): cv.uint8_t,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_device_type(config[CONF_DEVICE_TYPE]))
    cg.add(var.set_ct_version(config[CONF_CT_VERSION]))
    if CONF_FLOW_CONTROL_PIN in config:
        pin = await gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
    for conf in config.get(CONF_ON_CONTROL_COMMAND, []):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID],
            var,
            conf[CONF_CONTROL_COMMAND],
            conf[CONF_TARGET_DEVICE_TYPE],
        )
        await automation.build_automation(
            trigger,
            [(ComfortnetCommandData, "data"), (ComfortnetPointer, "client")],
            conf,
        )
    for conf in config.get(CONF_ON_PACKET, []):
        trigger = cg.new_Pvariable(
            conf[CONF_TRIGGER_ID],
            var,
            conf[CONF_PACKET_TYPE],
            conf[CONF_TARGET_DEVICE_TYPE],
            conf[CONF_REGISTER_PACKET_POLL],
            conf[CONF_PACKET_POLL_ONCE],
        )
        await automation.build_automation(
            trigger,
            [(ComfortnetPacketData, "data"), (ComfortnetPointer, "client")],
            conf,
        )
