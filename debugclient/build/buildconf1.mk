
all: buildmk.stamp generated_list
	@$(MAKE) -s -f build/buildconf2.mk

buildmk.stamp: build/buildcheck.sh
	@build/buildcheck.sh $@
        
generated_list:
	@echo config_m4_files = >> $@
