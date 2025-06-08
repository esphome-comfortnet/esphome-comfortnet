import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_SENSOR_DATAPOINT

from .. import (
    CONF_COMFORTNET_ID,
    COMFORTNET_CLIENT_SCHEMA,
    CONF_SENSOR_KEY,
    CONF_TARGET_DEVICE_TYPE,
    ComfortnetClient,
    comfortnet_ns,
)

DEPENDENCIES = ["comfortnet"]

ComfortnetBinarySensor = comfortnet_ns.class_(
    "ComfortnetBinarySensor", binary_sensor.BinarySensor, cg.Component, ComfortnetClient
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(ComfortnetBinarySensor)
    .extend(COMFORTNET_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)

    paren = await cg.get_variable(config[CONF_COMFORTNET_ID])
    cg.add(var.set_comfortnet_parent(paren))
    cg.add(var.set_sensor_key(config[CONF_SENSOR_KEY]))
    cg.add(var.set_sensor_target_device_type(config[CONF_TARGET_DEVICE_TYPE]))
