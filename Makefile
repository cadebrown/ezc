###                     EZC Makefile v@VERSION@
#
#  EZC is free software; you are free to modify and/or redistribute it under the terms of the GNU GPLv3. See 'LICENSE' for more details.
#
#  To make requirements, run `make req`
#
#  Also included are other requirements for running ezc, such as python, gcc, tcc, etc.
#
#  To build these, run `make req/TARGET` where TARGET is the software you want to install:
#
#  See req/Makefile for a full list of requirement targets.
#
###

req_dir=req

$(req_dir)/%: FORCE
	$(MAKE) $(subst $(req_dir)/,,$@) -C $(req_dir)


$(req_dir): FORCE
	$(MAKE) mpfr -C $@

FORCE:
