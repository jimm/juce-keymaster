PROJ_DIR    = Builds/MacOSX
APP_DIR     = $(PROJ_DIR)/build/Debug
APP_TARGET  = KeyMaster - App
TEST_TARGET = KeyMasterTests - ConsoleApp
APP_PROJECT = $(PROJ_DIR)/KeyMaster.xcodeproj
TEST_PROJECT = $(PROJ_DIR)/KeyMasterTests.xcodeproj
CONFIG      = Debug

.PHONY: build
build:
	xcodebuild -project $(APP_PROJECT) -target $(APP_TARGET) \
	    -configuration $(CONFIG) build

.PHONY: build-tests
build-tests:
	xcodebuild -project $(TEST_PROJECT) -target $(TEST_TARGET) \
	    -configuration $(CONFIG) build

.PHONY: test
test: build-tests
	./test.sh

.PHONY: run
run:
	open "$(APP_DIR)/KeyMaster.app"

.PHONY: tags
tags:
	find Source -name '*.cpp' -o -name '*.h' | xargs etags

.PHONY: rm-prefs
rm-prefs:
	rm -f ~/Library/Application\ Support/KeyMaster.props

.PHONY: clean
clean:
	xcodebuild -project $(APP_PROJECT) -target $(APP_TARGET) \
	    -configuration $(CONFIG) clean
	xcodebuild -project $(TEST_PROJECT) -target $(TEST_TARGET) \
	    -configuration $(CONFIG) clean
