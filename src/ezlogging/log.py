###                     EZC src/ezlogging/log.py v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  Colorized output for EZC
#
#  TODO:
#    * Update for the new AST system (general cleanup)
#    * Implement some of the old error printing (see https://github.com/ChemicalDevelopment/ezc/blob/f125dacaee53ddcb97b6f1c4cf0c805dde925f12/src/parsing/__init__.py)
#
###

import ezlogging
import sys

def print_title(title, color=ezlogging.CYAN):
	if title != "":
		sys.stdout.write(color + title)

def print_message(message, color=ezlogging.DEFAULT, indent=1):
	if message != "":
		for i in range(0, indent):
			sys.stdout.write("  ")
		sys.stdout.write(color + message)
		sys.stdout.write("\n")
		

def print_messages(messages, color=ezlogging.DEFAULT, indent=1):
	if isinstance(messages, list):
		newmessages = []
		for msglines in messages:
			for msg in msglines.split('\n'):
				newmessages.append(msg)
		messages = newmessages
	else:
		if len(messages.split("\n")) > 1:
			newmessages = []
			for msg in messages.split('\n'):
				newmessages.append(msg)
			messages = newmessages
	if isinstance(messages, list):
		print_reset()
		for msg in messages:
			print_message(msg, color, indent)
	else:
		sys.stdout.write(color + ": " + messages)

def print_reset():
	sys.stdout.write("\n")
	sys.stdout.write(ezlogging.RESET)

def print_base(title, message, colors=[ezlogging.CYAN, ezlogging.DEFAULT]):
	print_title(title, colors[0])
	print_messages(message, colors[1])
	print_reset()

def err(title, message):
	print_base(title, message, [ezlogging.RED + ezlogging.BOLD + ezlogging.BLINK, ezlogging.RBOLD + ezlogging.LGRAY])

def warn(title, message):
	if ezlogging.verbosity >= 1:
		print_base(title, message, [ezlogging.LYELLOW + ezlogging.BOLD, ezlogging.RBOLD + ezlogging.DGRAY])

def info(title, message):
	if ezlogging.verbosity >= 2:
		print_base(title, message, [ezlogging.BLUE + ezlogging.BOLD, ezlogging.RBOLD + ezlogging.DEFAULT])

def extra(title, message):
	if ezlogging.verbosity >= 4:
		print_base(title, message, [ezlogging.CYAN, ezlogging.DIM + ezlogging.DEFAULT])

def init(verbose):
	ezlogging.verbosity = verbose
	info(ezlogging.name, ezlogging.version)
	extra("Date", ezlogging.date)
	extra("Authors", ezlogging.authors)

