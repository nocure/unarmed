# ww 2013-0326 creaded.

.phony: clean

SOURCE= crd_def.h
TARGETS= crd01 crd02

all: $(addsuffix .h,$(TARGETS))

TARGETS_BIN=$(addsuffix .bin,$(TARGETS))

clean:
	@rm -f $(TARGETS_BIN) $(addsuffix .h,$(TARGETS))

%.h: %.bin
	@bin/bin2h $<

$(TARGETS_BIN): $(SOURCE)
	@bin/mkcrd $<


