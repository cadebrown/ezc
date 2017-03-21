###                     EZC src/ezlogging/__init__.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Colorized output for EZC
#
#  TODO:
#    * Cleanup/See if python has anything for colors builtin
#
###

name = "EZCC"

version = "@VERSION@"

if version.startswith("@"):
	version = "dev"

import time
#time.strftime("%Y-%m-%d %H:%M:%S %z")
date=time.strftime("%Y-%m-%d %H:%M:%S %z")

authors=["Cade Brown <cade@cade.site>"]



WHITE = '\033[97m'

BLACK = '\033[30m'
BLACKB = '\033[40m'

RED = '\033[31m'
REDB = '\033[41m'

LRED = '\033[91m'
LREDB = '\033[101m'

GREEN = '\033[32m'
GREENB = '\033[42m'

LGREEN = '\033[92m'
LGREENB = '\033[102m'

YELLOW = '\033[33m'
YELLOWB = '\033[43m'

LYELLOW = '\033[93m'
LYELLOWB = '\033[103m'

BLUE = '\033[34m'
BLUEB = '\033[44m'

LBLUEB = '\033[94m'
LBLUEB = '\033[104m'

MAGENTA = '\033[35m'
MAGENTAB = '\033[45m'

LMAGENTA = '\033[95m'
LMAGENTAB = '\033[105m'

CYAN = '\033[36m'
CYANB = '\033[46m'

LCYAN = '\033[96m'
LCYANB = '\033[106m'

LGRAY = '\033[37m'
LGRAYB = '\033[47m'

DGRAY = '\033[90m'
DGRAYB = '\033[100m'

BOLD = '\033[1m'
RBOLD = '\033[21m'

DIM = '\033[2m'
RDIM = '\033[22m'

UNDERLINE = '\033[4m'
RUNDERLINE = '\033[24m'

BLINK = '\033[5m'
RBLINK = '\033[25m'

INVERT = '\033[7m'
RINVERT = '\033[27m'

HIDE = '\033[8m'
RHIDE = '\033[28m'

DEFAULT = LGRAY

RESET = '\033[0m' + DEFAULT


verbosity = 2
silent = False


