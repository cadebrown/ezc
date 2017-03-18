
req_dir=req

$(req_dir)/%: FORCE
	$(MAKE) $(subst $(req_dir)/,,$@) -C $(req_dir)


$(req_dir): FORCE
	$(MAKE) mpfr -C $@

FORCE:
