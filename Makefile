DOCKER_IMAGE = msdos-builder
MSDOS_CC = ia16-elf-gcc
# The active gcc-ia16 toolchain does not provide -mcmodel=large.
# Medium gives far code calls and is the available Phase 1 step toward
# the large-model direction in design/design_phase1_full.md.
MSDOS_CFLAGS = -std=gnu89 -Wall -Wextra -Os -march=i8086 -mtune=i8086 -mmsdos -mcmodel=medium -DXTOS_DEBUG
MSDOS_TINY_CFLAGS = -std=gnu89 -Wall -Wextra -Os -march=i8086 -mtune=i8086 -mmsdos -mcmodel=tiny -DXTOS_DEBUG
BUILD_DIR = build
INPUT ?= XTOS.CGA
OUTPUT ?= XTOS.png
MODE ?= 320
XTOS_COM = $(BUILD_DIR)/xtos.com
RUNTIME_EXE = $(BUILD_DIR)/runtime.exe
FONT_EXE = $(BUILD_DIR)/font.exe
CONTROL_EXE = $(BUILD_DIR)/control.exe
SHOWCASE_EXE = $(BUILD_DIR)/showcase.exe
SMOKE_EXE = $(BUILD_DIR)/smoke.exe
FONT_CGA = $(BUILD_DIR)/font.cga
CONTROL_CGA = $(BUILD_DIR)/control.cga
SHOWCASE_CGA = $(BUILD_DIR)/showcase.cga
SMOKE_CGA = $(BUILD_DIR)/smoke.cga
FONT_PNG = $(BUILD_DIR)/font.png
CONTROL_PNG = $(BUILD_DIR)/control.png
SHOWCASE_PNG = $(BUILD_DIR)/showcase.png
SMOKE_PNG = $(BUILD_DIR)/smoke.png
RUNTIME_SOURCES = runtime/app.c runtime/client.c runtime/api.c runtime/int60.c runtime/event.c runtime/display.c runtime/system.c runtime/log.c runtime/screenshot.c runtime/text.c runtime/font.c runtime/invalidation.c runtime/mouse.c runtime/cursor.c runtime/ui/form.c runtime/ui/label.c runtime/ui/list.c runtime/ui/button.c runtime/screen.c runtime/draw.c runtime/draw_cga.c runtime/draw_text.c runtime/int60.s runtime/int60_call.s runtime/cga.s runtime/keyboard.s runtime/mouse.s
RESIDENT_SOURCES = runtime/resident.c runtime/resident_dispatch.s runtime/resident_services.c runtime/resident_log.c runtime/resident_dos.s runtime/client.c runtime/canvas.c runtime/draw.c runtime/draw_cga.c runtime/draw_text.c runtime/font.c runtime/screen.c runtime/int60.s runtime/int60_call.s runtime/tsr.s runtime/cga.s $(FONT_SOURCES)
BOOT_SOURCES = boot/xtos.c runtime/client.c boot/int60_call_tiny.s boot/exec_tiny.s
FONT_SOURCES = runtime/fonts/font_4x6.c runtime/fonts/font_5x7.c runtime/fonts/font_5x8.c
FONT_VIEWER_SOURCES = $(FONT_SOURCES)

.PHONY: run-font run-control run-showcase run-smoke validate-build cga-png cga-bmp font-png control-png showcase-png smoke-png clean docker-image

docker-image:
	docker build --platform linux/amd64 -t $(DOCKER_IMAGE) -f Dockerfile.msdos .

$(BUILD_DIR):
	mkdir -p $@

$(XTOS_COM): docker-image $(BOOT_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_TINY_CFLAGS) -I . -o $@ $(BOOT_SOURCES)

$(RUNTIME_EXE): docker-image $(RESIDENT_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -DXTOS_BUILD_RUNTIME -I . -o $@ $(RESIDENT_SOURCES)

$(FONT_EXE): docker-image apps/font.c $(RUNTIME_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/font.c $(RUNTIME_SOURCES)

$(CONTROL_EXE): docker-image apps/control.c $(RUNTIME_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/control.c $(RUNTIME_SOURCES)

$(SHOWCASE_EXE): docker-image apps/showcase.c $(RUNTIME_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/showcase.c $(RUNTIME_SOURCES)

$(SMOKE_EXE): docker-image apps/smoke.c $(RUNTIME_SOURCES) | $(BUILD_DIR)
	docker run --rm --platform linux/amd64 -v $(shell pwd):/project $(DOCKER_IMAGE) \
		$(MSDOS_CC) $(MSDOS_CFLAGS) -I . -o $@ apps/smoke.c $(RUNTIME_SOURCES)

run-font: $(XTOS_COM) $(RUNTIME_EXE) $(FONT_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec -c "mount c ." -c "c:" -c "cd build" -c "xtos.com font.exe" -c "exit"

run-control: $(XTOS_COM) $(RUNTIME_EXE) $(CONTROL_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec -c "mount c ." -c "c:" -c "cd build" -c "xtos.com control.exe" -c "exit"

run-showcase: $(XTOS_COM) $(RUNTIME_EXE) $(SHOWCASE_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec -c "mount c ." -c "c:" -c "cd build" -c "xtos.com showcase.exe" -c "exit"

run-smoke: $(XTOS_COM) $(RUNTIME_EXE) $(SMOKE_EXE)
	/Applications/DOSBox\ Staging.app/Contents/MacOS/dosbox -noautoexec -c "mount c ." -c "c:" -c "cd build" -c "xtos.com smoke.exe" -c "exit"

validate-build: $(XTOS_COM) $(RUNTIME_EXE) $(FONT_EXE) $(CONTROL_EXE) $(SHOWCASE_EXE) $(SMOKE_EXE)

cga-png:
	tools/cga2bmp.py $(INPUT) $(OUTPUT) --mode $(MODE)

cga-bmp:
	tools/cga2bmp.py $(INPUT) $(OUTPUT) --mode $(MODE)

font-png: $(FONT_PNG)

control-png: $(CONTROL_PNG)

showcase-png: $(SHOWCASE_PNG)

smoke-png: run-smoke
	tools/cga2bmp.py $(SMOKE_CGA) $(SMOKE_PNG) --mode 320

$(FONT_PNG): $(FONT_CGA)
	tools/cga2bmp.py $(FONT_CGA) $@ --mode 320

$(CONTROL_PNG): $(CONTROL_CGA)
	tools/cga2bmp.py $(CONTROL_CGA) $@ --mode 320

$(SHOWCASE_PNG): $(SHOWCASE_CGA)
	tools/cga2bmp.py $(SHOWCASE_CGA) $@ --mode 320

clean:
	rm -rf $(BUILD_DIR)
