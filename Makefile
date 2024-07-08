PROJ_DIR = Builds/MacOSX
APP_DIR = $(PROJ_DIR)/build/Debug
SCHEME = 'KeyMaster - App'

.PHONY: build
build:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) build

.PHONY: test
test:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) build-for-testing

.PHONY: run
run:
	open $(APP_DIR)/KeyMaster.app

.PHONY: rm-prefs
rm-prefs:
	rm -f "~/Library/Application Support/KeyMaster.props"
