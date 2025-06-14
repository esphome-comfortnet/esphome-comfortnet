#!/usr/bin/env bash
# Set up ESPHome dev environment

set -e

cd "$(dirname "$0")/.."
if [ ! -n "$VIRTUAL_ENV" ]; then
  if [ -x "$(command -v uv)" ]; then
    uv venv venv
  else
    python3 -m venv venv
  fi
  source venv/bin/activate
fi

if ! [ -x "$(command -v uv)" ]; then
  python3 -m pip install uv
fi

uv pip install setuptools wheel
# uv pip install -e ".[dev,test]" --config-settings editable_mode=compat

pre-commit install

script/platformio_install_deps.py platformio.ini --libraries --tools --platforms

mkdir -p .temp

echo
echo
echo "Virtual environment created. Run 'source venv/bin/activate' to use it."
