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

src_dir=src
req_dir=req
bundle_dir=bundle

inc=


$(req_dir)/%: FORCE
	$(MAKE) $(subst $(req_dir)/,,$@) -C $(req_dir)


$(bundle_dir): FORCE
	rm -rf $(bundle_dir)
	mkdir -p $(bundle_dir)
	mkdir -p $(bundle_dir)/$(req_dir)
	mkdir -p $(bundle_dir)/$(req_dir)/bin
	cp -Rf $(src_dir) $(bundle_dir)/$(src_dir)
	find $(bundle_dir) -regex "\(.*__pycache__.*\|*.py[co]\)" -delete
	
	for sreq in $(inc) ; do \
	    cp $$sreq $(bundle_dir)/$$sreq ; \
	done

	cp ezcc.py $(bundle_dir)/ezcc


$(req_dir): FORCE
	$(MAKE) mpfr -C $@

FORCE:
