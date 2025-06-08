import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import (
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_NUMBER_DATAPOINT,
    CONF_STEP,
)

from .. import (
    CONF_COMFORTNET_ID,
    COMFORTNET_CLIENT_SCHEMA,
    CONF_SENSOR_KEY,
    CONF_TARGET_DEVICE_TYPE,
    ComfortnetClient,
    comfortnet_ns,
)

DEPENDENCIES = ["comfortnet"]

ComfortnetNumber = comfortnet_ns.class_(
    "ComfortnetNumber", number.Number, cg.Component, ComfortnetClient
)


def validate_min_max(config):
    if config[CONF_MAX_VALUE] <= config[CONF_MIN_VALUE]:
        raise cv.Invalid("max_value must be greater than min_value")
    return config


CONFIG_SCHEMA = cv.All(
    number.number_schema(ComfortnetNumber)
    .extend(
        {
            cv.Required(CONF_MAX_VALUE): cv.float_,
            cv.Required(CONF_MIN_VALUE): cv.float_,
            cv.Required(CONF_STEP): cv.positive_float,
        }
    )
    .extend(COMFORTNET_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA),
    validate_min_max,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await number.register_number(
        var,
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP],
    )

    paren = await cg.get_variable(config[CONF_COMFORTNET_ID])
    cg.add(var.set_comfortnet_parent(paren))
    cg.add(var.set_sensor_key(config[CONF_SENSOR_KEY]))
    cg.add(var.set_sensor_target_device_type(config[CONF_TARGET_DEVICE_TYPE]))
