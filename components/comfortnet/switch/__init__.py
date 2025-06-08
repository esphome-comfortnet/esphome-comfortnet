import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_SWITCH_DATAPOINT

from .. import (
    CONF_COMFORTNET_ID,
    CONF_DEVICE_ADDRESS,
    CONF_REQUEST_ONCE,
    CONF_SRC_ADDRESS,
    COMFORTNET_CLIENT_SCHEMA,
    ComfortnetClient,
    comfortnet_ns,
)

DEPENDENCIES = ["comfortnet"]

ComfortnetSwitch = comfortnet_ns.class_(
    "ComfortnetSwitch", switch.Switch, cg.Component, ComfortnetClient
)

CONFIG_SCHEMA = (
    switch.switch_schema(ComfortnetSwitch)
    .extend(
        {
            cv.Required(CONF_SWITCH_DATAPOINT): cv.string,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(COMFORTNET_CLIENT_SCHEMA)
)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)

    paren = await cg.get_variable(config[CONF_COMFORTNET_ID])
    cg.add(var.set_comfortnet_parent(paren))
    cg.add(var.set_request_mod(config[CONF_DEVICE_ADDRESS]))
    cg.add(var.set_request_once(config[CONF_REQUEST_ONCE]))
    cg.add(var.set_switch_id(config[CONF_SWITCH_DATAPOINT]))
    cg.add(var.set_src_adr(config[CONF_SRC_ADDRESS]))
