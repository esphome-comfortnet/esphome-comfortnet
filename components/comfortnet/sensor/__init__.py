import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_SENSOR_DATAPOINT

from .. import (
    CONF_COMFORTNET_ID,
    COMFORTNET_CLIENT_SCHEMA,
    CONF_SENSOR_KEY,
    CONF_TARGET_DEVICE_TYPE,
    ComfortnetClient,
    comfortnet_ns,
)

DEPENDENCIES = ["comfortnet"]

ComfortnetSensor = comfortnet_ns.class_(
    "ComfortnetSensor", sensor.Sensor, cg.Component, ComfortnetClient
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(ComfortnetSensor)
    .extend(COMFORTNET_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    paren = await cg.get_variable(config[CONF_COMFORTNET_ID])
    cg.add(var.set_comfortnet_parent(paren))
    cg.add(var.set_sensor_key(config[CONF_SENSOR_KEY]))
    cg.add(var.set_sensor_target_device_type(config[CONF_TARGET_DEVICE_TYPE]))
