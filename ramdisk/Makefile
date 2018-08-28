# ww 2013-0401 created.
# ww 2018-0604 modified. added PREFIX
.phony: clean


SOURCE = vfat_256k.ima
TARGET = ramdisk.a
PREFIX = arm-none-eabi-


TARGET_DEBUG= $(addprefix ../lib/Debug/lib, $(TARGET))
TARGET_RELEASE= $(addprefix ../lib/Release/lib, $(TARGET))
OBJECT = $(addsuffix .o, $(basename $(SOURCE)))


all: $(TARGET_DEBUG) $(TARGET_RELEASE)


$(TARGET_DEBUG): $(TARGET)
	cp $< $@

$(TARGET_RELEASE): $(TARGET)
	cp $< $@

$(TARGET): vfat.o
	$(PREFIX)ar -r $@ $<
	rm $<


vfat.ima: $(SOURCE)
	cp $< $@


%.o: %.ima
	$(PREFIX)objcopy -Ibinary -Oelf32-littlearm -Barm $< $@
	$(PREFIX)objdump -t $@
	rm $<


clean:
	rm -f $(TARGET)

