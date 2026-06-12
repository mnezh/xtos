DOCKER_IMAGE = msdos-builder
MSDOS_CC = ia16-elf-gcc
# The active gcc-ia16 toolchain does not provide -mcmodel=large.
# Medium gives far code calls and is the available Phase 1 step toward
# the large-model direction in design/design_phase1_full.md.
MSDOS_CFLAGS = -std=gnu89 -Wall -Wextra -Os -march=i8086 -mtune=i8086 -mmsdos -mcmodel=medium -DXTOS_DEBUG
BUILD_DIR = build
INPUT ?= XTOS.CGA
OUTPUT ?= XTOS.png
MODE ?= 320
FONT_EXE = $(BUILD_DIR)/font.exe
CONTROL_EXE = $(BUILD_DIR)/control.exe
SHOWCASE_EXE = $(BUILD_DIR)/showcase.exe
RUNTIME_SOURCES = runtime/app.c runtime/api.c runtime/int60.c runtime/event.c runtime/display.c runtime/system.c runtime/log.c runtime/screenshot.c runtime/text.c runtime/font.c runtime/canvas.c runtime/invalidation.c runtime/mouse.c runtime/cursor.c runtime/ui/form.c runtime/ui/label.c runtime/ui/list.c runtime/ui/button.c runtime/screen.c runtime/draw.c runtime/draw_cga.c runtime/draw_text.c runtime/int60.s runtime/cga.s runtime/keyboard.s runtime/mouse.s
FONT_SOURCES = runtime/fonts/font_4x6.c runtime/fonts/font_5x7.c runtime/fonts/font_5x8.c
FONT_VIEWER_SOURCES = $(FONT_SOURCES)

.PHONY: run-font run-control run-showcase validate-build cga-png cga-bmp clean docker-image

docker-image:
	docker build --platform linux/amd64 -t $(DOCKER_IMAGE) -f Dockerfile.msdos .

$(BUILD_DIR):
	mkdir -p $@

$(FONT_EXE): docker-image apps/font.c $(RUNTIME_SOURCES) $(FONT_VIEWER_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/font.c $(RUNTIME_SOURCES) $(FONT_VIEWER_SOURCES)

$(CONTROL_EXE): docker-image apps/control.c $(RUNTIME_SOURCES) $(FONT_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/control.c $(RUNTIME_SOURCES) $(FONT_SOURCES)

$(SHOWCASE_EXE): docker-image apps/showcase.c $(RUNTIME_SOURCES) $(FONT_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/showcase.c $(RUNTIME_SOURCES) $(FONT_SOURCES)

run-font: $(FONT_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec $(FONT_EXE)

run-control: $(CONTROL_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec $(CONTROL_EXE)

run-showcase: $(SHOWCASE_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec $(SHOWCASE_EXE)

validate-build: $(FONT_EXE) $(CONTROL_EXE) $(SHOWCASE_EXE)

cga-png:
	tools/cga2bmp.py $(INPUT) $(OUTPUT) --mode $(MODE)

cga-bmp:
	tools/cga2bmp.py $(INPUT) $(OUTPUT) --mode $(MODE)

clean:
	rm -rf $(BUILD_DIR)
