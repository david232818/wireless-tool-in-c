# Makefile

SUBDIRS = lib wlt main
OBJDIR = ./obj
EXENAME = ./bin/wltool

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	rm -f $(OBJDIR)/*.o $(EXENAME)

