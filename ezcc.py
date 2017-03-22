#!/usr/bin/env python
import sys
import os 
dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append("{0}/src".format(dir_path))

from src import ezc
ezc.main(sys.argv[1:])
