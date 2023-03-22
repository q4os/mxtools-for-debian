
PNG_DIR := .
SVG_DIR := .
BUILD   := ./build

SVGS = $(wildcard $(SVG_DIR)/*.svg)
PNGS = $(wildcard $(PNG_DIR)/*.png)

HICOLOR := $(BUILD)/icons/hicolor
HICO32  := $(HICOLOR)/32x32/apps

APPS_DIR = $(HICOLOR)/$${SIZE}x$${SIZE}/apps

ICONS = $(notdir $(basename $(SVGS)))

SIZES := 8 16 22 24 32 48 64 96 128 256

all: clean dirs pngs svgs svg2png  links

dirs:
	@echo make icons ...
	@for SIZE in $(SIZES); do mkdir -p $(APPS_DIR); done  
	@mkdir -p $(HICOLOR)/scalable

svgs:
	@for SVG in $(SVGS); do \
		echo "  " $(HICOLOR)/scalable/$$(basename $${SVG}); \
		cp $${SVG} $(HICOLOR)/scalable; \
		done  

pngs:
	@for PNG in $(PNGS); do \
		echo "  " $(HICO32)/$$(basename $${PNG}); \
		cp $${PNG} $(HICO32); done  

svg2png:
	@for SIZE in $(SIZES); do for ICON in $(ICONS); do \
		echo "  " $(APPS_DIR)/$${ICON}.png; \
		inkscape -h $${SIZE} -w $${SIZE} -o $(APPS_DIR)/$${ICON}.png $(SVG_DIR)/$${ICON}.svg 2>/dev/null; \
		done; done

links:
	@for SIZE in $(SIZES); do \
		ln -s mnotify-some-wireframe.png $(APPS_DIR)/apt-notifier.png; \
		ln -s mnotify-some-wireframe.png $(APPS_DIR)/mx-updater.png; \
		done
	@cd $(HICOLOR)/scalable && \
		ln -s mnotify-some-wireframe.svg apt-notifier.svg && \
		ln -s mnotify-some-wireframe.svg mx-updater.svg
	@cd $(BUILD)/icons && \
		ln -s hicolor/32x32/apps/*.png . && \
		ln -s hicolor/256x256/apps/mnotify-some-wireframe.png mnotify-some-wireframe-pkexec.png 

clean:
	-@rm -rf $(BUILD)

.PHONY: all dirs pngs svg2png svgs links clean
