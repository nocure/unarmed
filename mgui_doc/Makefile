
SOURCE = mgui
CHM_FILE=$(SOURCE).chm
TARGET = ../lib/$(CHM_FILE)

all: $(TARGET)

clean:
	rm -f *tmp

$(CHM_FILE): Doxyfile
	./doxygen
	rm -fr $(SOURCE)

$(TARGET): $(CHM_FILE)
	mv $< $@
