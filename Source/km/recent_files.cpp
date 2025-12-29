#include "recent_files.h"

RecentFiles::RecentFiles(ApplicationProperties* props)
    : app_properties_(props)
{
    load();
}

void RecentFiles::addFile(const juce::File& file) {
    if (!file.existsAsFile())
        return;

    juce::String path = file.getFullPathName();

    // Remove if already in list (to move it to top)
    int index = recent_files_.indexOf(path);
    if (index >= 0)
        recent_files_.remove(index);

    // Add to front of list
    recent_files_.insert(0, path);

    // Trim to max size
    while (recent_files_.size() > MAX_RECENT_FILES)
        recent_files_.remove(recent_files_.size() - 1);

    save();
}

void RecentFiles::removeFile(const juce::File& file) {
    juce::String path = file.getFullPathName();
    int index = recent_files_.indexOf(path);
    if (index >= 0) {
        recent_files_.remove(index);
        save();
    }
}

void RecentFiles::clear() {
    recent_files_.clear();
    save();
}

juce::StringArray RecentFiles::getRecentFiles(bool validateExistence) {
    if (validateExistence) {
        bool changed = false;
        for (int i = recent_files_.size() - 1; i >= 0; --i) {
            juce::File file(recent_files_[i]);
            if (!file.existsAsFile()) {
                recent_files_.remove(i);
                changed = true;
            }
        }
        if (changed)
            save();
    }
    return recent_files_;
}

void RecentFiles::load() {
    recent_files_.clear();

    if (app_properties_ != nullptr) {
        auto* props = app_properties_->getUserSettings();
        if (props != nullptr) {
            juce::String stored = props->getValue(RECENT_FILES_PROPERTY);
            if (stored.isNotEmpty()) {
                recent_files_.addTokens(stored, "|", "");
                // Trim to max size in case it was modified externally
                while (recent_files_.size() > MAX_RECENT_FILES)
                    recent_files_.remove(recent_files_.size() - 1);
            }
        }
    }
}

void RecentFiles::save() {
    if (app_properties_ != nullptr) {
        auto* props = app_properties_->getUserSettings();
        if (props != nullptr) {
            juce::String value = recent_files_.joinIntoString("|");
            props->setValue(RECENT_FILES_PROPERTY, value);
            app_properties_->saveIfNeeded();
        }
    }
}
