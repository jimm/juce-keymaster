#pragma once

#include <JuceHeader.h>

#define RECENT_FILES_PROPERTY "km.recent_files"

class RecentFiles {
public:
    RecentFiles(ApplicationProperties* props);

    void addFile(const juce::File& file);
    void removeFile(const juce::File& file);
    void clear();
    juce::StringArray getRecentFiles(bool validateExistence = true);

private:
    void load();
    void save();
    ApplicationProperties* app_properties_;
    juce::StringArray recent_files_;
    static constexpr int MAX_RECENT_FILES = 8;
};
