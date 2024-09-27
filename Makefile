PROJ_DIR = Builds/MacOSX
APP_DIR = $(PROJ_DIR)/build/Debug
SCHEME = 'KeyMaster - App'
DESTINATION = 'platform=macOS,name=Any Mac'

.PHONY: build
build:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) -destination $(DESTINATION) build

.PHONY: test
test:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) -destination $(DESTINATION) -quiet build-for-testing

.PHONY: run
run:
	open $(APP_DIR)/KeyMaster.app

.PHONY: tags
tags:
	find Source -name '*.cpp' -o -name '*.h' | xargs etags

.PHONY: rm-prefs
rm-prefs:
	rm -f "~/Library/Application Support/KeyMaster.props"
