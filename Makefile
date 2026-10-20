.PHONY: help sdk stock nextui nxredux all-roms source stock-source setup desktop run doctor spotify soundcloud
help:
	@printf '%s\n' 'NXT Retro Music v1.9.3 — shared source for all ROMs' 'make stock / nextui / nxredux: native installer + OTA kit' 'make all-roms: build all three, sequentially' 'make sdk: download pinned ARM64 SDK' 'make source: package verified common source' 'Read BUILD_v1.9.3.md first.'
sdk:
	python3 tools/sdk-assets.py
stock:
	python3 tools/build-release.py --rom stock
nextui:
	python3 tools/build-release.py --rom nextui
nxredux:
	python3 tools/build-release.py --rom nxredux
all-roms:
	python3 tools/build-release.py --rom all
source:
	python3 tools/sync-overlay.py
	python3 tools/package-source.py
stock-source: source
setup:
	bash bootstrap-wsl.sh
desktop:
	bash tools/build-native.sh desktop
run:
	bash tools/run-desktop.sh
doctor:
	bash tools/doctor.sh
spotify:
	bash tools/build-spotify.sh
soundcloud:
	python3 tools/setup-soundcloud-runtime.py
	bash tools/build-soundcloud.sh
