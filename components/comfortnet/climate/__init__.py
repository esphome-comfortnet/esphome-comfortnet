import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_CUSTOM_FAN_MODES, CONF_CUSTOM_PRESETS, CONF_ID

from .. import (
    CONF_COMFORTNET_ID,
    COMFORTNET_CLIENT_SCHEMA,
    ComfortnetClient,
    comfortnet_ns,
)

DEPENDENCIES = ["comfortnet"]

ComfortnetClimate = comfortnet_ns.class_(
    "ComfortnetClimate", climate.Climate, cg.Component, ComfortnetClient
)


def ensure_climate_mode_map(value):
    cv.check_not_templatable(value)
    options_map_schema = cv.Schema({cv.uint8_t: climate.validate_climate_mode})
    value = options_map_schema(value)
    all_values = list(value.keys())
    unique_values = set(value.keys())
    if len(all_values) != len(unique_values):
        raise cv.Invalid("Mapping values must be unique.")
    return value


def ensure_option_map(value):
    cv.check_not_templatable(value)
    options_map_schema = cv.Schema({cv.uint8_t: cv.string_strict})
    value = options_map_schema(value)
    all_values = list(value.keys())
    unique_values = set(value.keys())
    if len(all_values) != len(unique_values):
        raise cv.Invalid("Mapping values must be unique.")
    return value


CONFIG_SCHEMA = cv.All(
    climate.CLIMATE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ComfortnetClimate),
            cv.Optional(CONF_CURRENT_TEMPERATURE_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_TARGET_TEMPERATURE_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_TARGET_TEMPERATURE_LOW_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_TARGET_TEMPERATURE_HIGH_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_MODE_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_CUSTOM_PRESET_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_CUSTOM_FAN_MODE_DATAPOINT, default=""): cv.string,
            cv.Optional(
                CONF_CUSTOM_FAN_MODE_NO_SCHEDULE_DATAPOINT, default=""
            ): cv.string,
            cv.Optional(CONF_FOLLOW_SCHEDULE_DATAPOINT, default=""): cv.string,
            cv.Optional(CONF_MODES, default={}): ensure_climate_mode_map,
            cv.Optional(CONF_CUSTOM_PRESETS, default={}): ensure_option_map,
            cv.Optional(CONF_CUSTOM_FAN_MODES, default={}): ensure_option_map,
            cv.Optional(CONF_CURRENT_HUMIDITY_DATAPOINT, default=""): cv.string,
            cv.Optional(
                CONF_TARGET_DEHUMIDIFICATION_LEVEL_DATAPOINT, default=""
            ): cv.string,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(COMFORTNET_CLIENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    paren = await cg.get_variable(config[CONF_COMFORTNET_ID])
    cg.add(var.set_comfortnet_parent(paren))
    cg.add(var.set_request_mod(config[CONF_DEVICE_ADDRESS]))
    cg.add(var.set_request_once(config[CONF_REQUEST_ONCE]))
    cg.add(var.set_src_adr(config[CONF_SRC_ADDRESS]))
    cg.add(var.set_current_temperature_id(config[CONF_CURRENT_TEMPERATURE_DATAPOINT]))
    cg.add(var.set_target_temperature_id(config[CONF_TARGET_TEMPERATURE_DATAPOINT]))
    cg.add(
        var.set_target_temperature_low_id(config[CONF_TARGET_TEMPERATURE_LOW_DATAPOINT])
    )
    cg.add(
        var.set_target_temperature_high_id(
            config[CONF_TARGET_TEMPERATURE_HIGH_DATAPOINT]
        )
    )
    cg.add(var.set_mode_id(config[CONF_MODE_DATAPOINT]))
    cg.add(var.set_custom_preset_id(config[CONF_CUSTOM_PRESET_DATAPOINT]))
    cg.add(var.set_custom_fan_mode_id(config[CONF_CUSTOM_FAN_MODE_DATAPOINT]))
    cg.add(
        var.set_custom_fan_mode_no_schedule_id(
            config[CONF_CUSTOM_FAN_MODE_NO_SCHEDULE_DATAPOINT]
        )
    )
    cg.add(var.set_follow_schedule_id(config[CONF_FOLLOW_SCHEDULE_DATAPOINT]))
    cg.add(
        var.set_modes(
            list(config[CONF_MODES].keys()),
            list(config[CONF_MODES].values()),
        )
    )
    cg.add(
        var.set_custom_presets(
            list(config[CONF_CUSTOM_PRESETS].keys()),
            list(config[CONF_CUSTOM_PRESETS].values()),
        )
    )
    cg.add(
        var.set_custom_fan_modes(
            list(config[CONF_CUSTOM_FAN_MODES].keys()),
            list(config[CONF_CUSTOM_FAN_MODES].values()),
        )
    )
    cg.add(var.set_current_humidity_id(config[CONF_CURRENT_HUMIDITY_DATAPOINT]))
    cg.add(
        var.set_target_dehumidification_level_id(
            config[CONF_TARGET_DEHUMIDIFICATION_LEVEL_DATAPOINT]
        )
    )
