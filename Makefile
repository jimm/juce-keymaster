PROJ_DIR = Builds/MacOSX
APP_DIR = $(PROJ_DIR)/build/Debug
SCHEME = 'KeyMaster - App'
DESTINATION = 'platform=macOS,name=Any Mac'
PROJECT=KeyMaster.xcodeproj
TEST_PROJECT=KeyMasterTests.xcodeproj

.PHONY: build
build:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) -destination $(DESTINATION) \
	    -project $(PROJECT) build

.PHONY: test
test:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) -destination $(DESTINATION) \
	    -project $(TEST_PROJECT) -quiet build-for-testing

.PHONY: run
run:
	open $(APP_DIR)/KeyMaster.app

.PHONY: tags
tags:
	find Source -name '*.cpp' -o -name '*.h' | xargs etags

.PHONY: rm-prefs
rm-prefs:
	rm -f "~/Library/Application Support/KeyMaster.props"
