# esphome-comfortnet [![Made for ESPHome](https://img.shields.io/badge/Made_for-ESPHome-black?logo=esphome)](https://esphome.io)

This [ESPHome](https://esphome.io) package enables local control of a ComfortNet compatible appliance, such as Daikin, Goodman and Amana HVAC, and creates entities in Home Assistant to control and monitor these devices. This package provides more detailed and reliable sensors than Daikin's (or other manufacturer's) cloud-based [DaikinSkyport](https://github.com/apetrycki/daikinskyport) available in HACS, and does so without any requirement for internet access by accessing the device either via its RJ11 diagnostics port, or via the 1/2/R/C thermostat connection, depending on device model.

This package and the DaikinSkyport integration can coexist if desired.

As this is heavily inspired by [ESPHome-Econet](https://github.com/esphome-econet/esphome-econet), much of the setup is the same, however Econet will need to be swapped with ComfortNet.

## Protocol Resources

[ComfortNet-HVAC-ESP32](https://github.com/smurf12345/home-assistant/tree/main/ComfortNet-HVAC-ESP32) debug script provided the initial start and proof of concept.
[Net485](https://github.com/kpishere/Net485) was an invaluable resource in getting an idea of a real implementation of the ClimateTalk protocol.
Thank you to [rrmayer's ClimateTalk documentation](https://github.com/rrmayer/climate-talk-web-api/tree/master/docs/spec) for being maybe the last place on the internet where old Climate Talk documentation is available.

## Supported Daikin Hardware

Theoretically, modern Daikin, Goodman and Amana HVAC Systems should work (and potentially other major brands that use a variant of ClimateTalk 2.0), however this is only tested on Daikin One+ so far.

## Required ESPHome Hardware

All that's needed to run ESPHome-ComfortNet is an ESP32 or ESP8266 microcontroller, an RS485 interface, either a phone cord or some wires and ingenuity to hook it up to your ComfortNet appliance and a USB-C charger to power it. For simplicity, we recommend the M5Stack K045 Kit, which includes both the ESP32 & RS485 components in a simple package.

Be warned that certain HVAC board models (like the Emerson PCBKF105) don't seem to have fully wired debug ports, and may require wiring straight to the thermostat lines, either by splicing them, or hooking into the connections (such as wirenuts) inside the HVAC.

ComfortNet is traditionally wired between equipment with Data 1, Data 2, R (24V AC), C (ground). In RS485 terms, Data 1 = A and Data 2 = B. PLEASE NOTE that power is supplied as 24V AC, NOT direct current, and is thus incompatible with most ESP hardware. You will need another way to power it, or a device that can take AC input. A 24VAC to 12VDC transformer may be an option.

Finally, ground is not strictly required for RS485, as it uses the differential pair to communicate rather than relative to ground, however it's often suggested to connect ground anyways for a variety of reasons.

For full details on what hardware to buy, head over to the [ESPHome-Econet Recommended Hardware Purchase and Setup on their wiki](https://github.com/esphome-econet/esphome-econet/wiki/Recommended-Hardware-Purchase-and-Setup-Instructions).

<img src="https://github.com/esphome-econet/econet-docs/blob/main/photos-and-screenshots/Correctly-Wired-K045.jpeg?raw=true" alt="A correctly wired K045 unit." width=25%>

## Diagnostic Port Wiring

Diagnostic RJ-11 plug, front view. Only 1/2 (A/B) are strictly necessary, however R/GND is also recommended.

```
             ┌─────────┐
          1 ─┼──       │
C (24VAC) 2 ─┼──       └──┐
2     (B) 3 ─┼──          │
1     (A) 4 ─┼──          │
R   (GND) 5 ─┼──       ┌──┘
          6 ─┼──       │
             └─────────┘
```

As defined in ClimateTalk Alliance CT2.0 CT-485 Physical Specification, Revision 01\
5.4.3 Diagnostic Mating Plug\
Figure 9 - Four-Pin RJ-11 Pin Assignment

As defined in 7.2 Transmission Bus Stability, it is generally recommended to connect the ground line between devices, however a 100Ω resistor to the ground connection may be desired for very long cable runs to an externally powered device, to prevent ground loops. More details are present in the documentation if needed.

## Device Framework

This has been built primarily with ESP-IDF in mind, however it will also run on the Arduino framework, albeit with some downsides. Persistent MAC addresses are not implemented at the moment, nor are they configurable, so the Arduino framework will randomly generate a new MAC at each restart. Additionally, random number generation on modern ESP32 chips should be significantly more random than Arduino PRNG.

## Software Installation

For alternative software installation methods and details on how to customize your configuration, check out [the ESPHome-Econet detailed Software Configuration and Installation Guide on their wiki](https://github.com/esphome-econet/esphome-econet/wiki/Initial-ESPHome%E2%80%90econet-Software-Configuration-and-Installation). Please make sure to replace any references to ESPHome-Econet with ESPHome-ComfortNet though!

## Contributing to ESPHome ComfortNet

Contributions to the ESPHome-comfortnet are welcome and encouraged! If you're looking to help out and need inspiration, please check out [the list of open issues](https://github.com/esphome-comfortnet/esphome-comfortnet/issues).

Contributors will want to install esphome and pre-commit, then install pre-commit hooks for your forked repo:

```bash
pip install -U esphome pre-commit  # Install required python packages
cd esphome-comfortnet  # CD into the directory where you cloned your forked esphome-comfortnet repo
pre-commit install  # Enable pre-commit hooks to run prior to any commit
```

### Testing Local Changes

The esphome CLI can be used to compile and install changes to YAML and/or code via the `esphome config|compile|run` commands. The provided `example-local.yaml` file provides a simple example of how to build with all local changes like this; just add a `secrets.yaml` file to the root of your checked-out repo and run `esphome compile example-local.yaml` to test compilation of your configuration and code changes. You can use the `esphome config example-local.yaml` command to see the results of any config updates, or the `esphome run example-local.yaml` command to deploy your changes to an ESPHome-capable device over Wi-Fi or USB.

## License

ESPHome-ComfortNet
Copyright (C) 2025  PseudoResonance

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
