PROJ_DIR = Builds/MacOSX
APP_DIR = $(PROJ_DIR)/build/Debug
SCHEME = 'KeyMaster - App'

.PHONY: build
build: schema
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) build

.PHONY: schema
schema: Source/km/schema.sql.h

# Turn db/schema.sql into a C++11 header file that defines a string
# containing the SQL.
Source/km/schema.sql.h: db/schema.sql
	@echo "// THIS FILE IS GENERATED FROM $<" > $@ \
	&& echo 'static const char * const SCHEMA_SQL = R"(' >> $@ \
	&& cat $< >> $@ \
	&& echo ')";' >> $@

.PHONY: test
test:
	cd $(PROJ_DIR) && xcodebuild -scheme $(SCHEME) build-for-testing

.PHONY: run
run:
	open $(APP_DIR)/KeyMaster.app

.PHONY: rm-prefs
rm-prefs:
	rm -f "~/Library/Application Support/KeyMaster.props"
