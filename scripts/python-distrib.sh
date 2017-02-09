#!/bin/sh

export PYTHONHOME=$(dirname $0)/src/
$(dirname $0)/src/python-bin $(dirname $0)/src/ezcc.py "${@}"
