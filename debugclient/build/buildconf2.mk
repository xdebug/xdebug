
include generated_list

TOUCH_FILES = mkinstalldirs install-sh missing

all: $(TOUCH_FILES) config_h.in aclocal.m4 targets configure

targets:
	@echo rebuilding Makefile templates
	@for i in `find . -name Makefile.am`; do \
		(automake --foreign  -a `dirname $$i`/Makefile 2>/dev/null || true); \
		echo "   `dirname $$i`/Makefile.in"; \
	done

aclocal.m4: configure.in acinclude.m4
	@echo rebuilding $@
	@aclocal 2>&1

config_h.in: configure

# explicitly remove target since autoheader does not seem to work 
# correctly otherwise (timestamps are not updated)
	@echo rebuilding $@
	@rm -f $@
	@autoheader 2>&1

$(TOUCH_FILES):
	@touch $(TOUCH_FILES)

configure: aclocal.m4 configure.in $(config_m4_files)
	@echo rebuilding $@
	@autoconf 2>&1 | (egrep -v 'warning: AC_PROG_LEX invoked multiple times' || true)
