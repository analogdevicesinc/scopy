TARGET := Scopy.flatpak
SRC := org.adi.Scopy.json

ifndef ARCH
	ARCH := x86_64
endif
ifndef EN_PREPROCESS
	EN_PREPROCESS := true
endif
# libiio API Scopy is built against: 1 (default) or 0. Each version keeps its own flatpak-builder cache.
ifndef SCOPY_LIBIIO_VERSION
	SCOPY_LIBIIO_VERSION := 1
endif
ifeq ($(SCOPY_LIBIIO_VERSION),0)
	LIBIIO_DEFINE := -DLIBIIO_V0
	STATE_DIR := .flatpak-builder-libiio-v0
else
	STATE_DIR := .flatpak-builder
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): repo
	flatpak build-bundle $< $@ org.adi.Scopy

repo: build
	flatpak build-export $@ $<

build: preprocess
	flatpak-builder --verbose --keep-build-dirs --ccache --user --disable-rofiles-fuse --disable-updates --state-dir=$(STATE_DIR) --arch=$(ARCH) $@ $(SRC)

preprocess: clean
	if [ $(EN_PREPROCESS) = true ]; then \
		if [ $(ARCH) = x86_64 ]; then \
			echo "-- Preprocessing org.adi.Scopy.json for ARCH = X86_64"; \
			gcc -E -P -D__X86__ $(LIBIIO_DEFINE) org.adi.Scopy.json.c -o org.adi.Scopy.json; \
		elif [ $(ARCH) = arm ]; then \
			echo "-- Preprocessing org.adi.Scopy.json for ARCH = arm"; \
			gcc -E -P -D__ARM__ $(LIBIIO_DEFINE) org.adi.Scopy.json.c -o org.adi.Scopy.json; \
		fi \
	fi

clean:
	rm -rf $(TARGET) build repo

