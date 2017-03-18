
req_dir=req

$(req_dir): FORCE
	$(MAKE) mpfr -C $@

FORCE:
