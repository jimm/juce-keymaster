# Implementation Plan: File -> Recent Menu Feature

## Overview

This document outlines the implementation plan for adding a "File -> Recent"
menu feature to KeyMaster. The feature will remember the previous 8 project
files opened and persist them in the application's user settings.

## Implementation Tasks

### 1. Add recent files storage to settings system

**Location:** `Source/MainComponent.cpp`

- Use JUCE's `ApplicationProperties` (already set up in `Main.cpp`)
- Add new settings key constant (similar to existing `KM_FILE_PROPERTY` at
  line 20)
- Store recent files as a delimited string (pipe-separated file paths)
- Maximum of 8 recent files

**Suggested constant:**
```cpp
#define RECENT_FILES_PROPERTY "km.recent_files"
```

### 2. Create RecentFiles helper class to manage recent file list

**New files:**
- `Source/km/recent_files.h`
- `Source/km/recent_files.cpp`

**Class responsibilities:**
- Load recent files from settings on startup
- Add file to list (moves to top, removes duplicates)
- Remove file from list (if deleted or invalid)
- Get list of recent files
- Save list back to settings
- Validate file existence before returning list
- Maintain maximum of 8 files in LRU (Least Recently Used) order

**Interface suggestion:**
```cpp
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
```

### 3. Add CommandID for recent file menu items in MenuManager

**Location:** `Source/gui/menu_manager.cpp`

- Add `clear_recent_files` command to `CommandIDs` enum (around line 14)
- Add `recent_file_base` with range for 8 files
  (e.g., `recent_file_base = 0x2300`)
- Register commands in `getAllCommands()` method (lines 111-172)
- Add command info in `getCommandInfo()` method (lines 174-286)

**Enum additions:**
```cpp
enum CommandIDs {
    // ... existing commands ...
    recent_file_base = 0x2300,
    recent_file_1 = recent_file_base,
    recent_file_2,
    recent_file_3,
    recent_file_4,
    recent_file_5,
    recent_file_6,
    recent_file_7,
    recent_file_8,
    clear_recent_files
};
```

### 4. Update File menu to include Recent Files submenu

**Location:** `Source/gui/menu_manager.cpp` (lines 398-457)

- Add "Recent Files" submenu after "Open Project" in File menu
- Dynamically populate submenu with recent files from RecentFiles class
- Show full paths in menu item text (or use shortened display names)
- Add separator and "Clear Recent Files" menu item at bottom
- Gray out/disable items if list is empty

**Target menu structure:**
```
File
  ├─ New Project
  ├─ Open Project...
  ├─ Recent Files →
  │    ├─ /path/to/project1.kmst
  │    ├─ /path/to/project2.kmst
  │    ├─ ...
  │    ├─ ───────────────
  │    └─ Clear Recent Files
  ├─ ─────────────
  ├─ Save Project
  └─ Save As...
```

### 5. Implement command handler for recent file selection

**Location:** `Source/gui/menu_manager.cpp`, `perform()` method
(lines 288-396)

- Handle `recent_file_base + index` commands
- Get selected file path from RecentFiles class
- Call `main_component_->loadDocument()` with the file path
- Handle `clear_recent_files` command to empty the list
- Add error handling if file no longer exists

**Implementation approach:**
```cpp
bool MenuManager::perform(const InvocationInfo& info) {
    // ... existing cases ...

    if (info.commandID >= recent_file_base &&
        info.commandID < recent_file_base + 8) {
        int index = info.commandID - recent_file_base;
        auto files = recent_files_->getRecentFiles();
        if (index < files.size()) {
            juce::File file(files[index]);
            main_component_->loadDocument(file);
        }
        return true;
    }

    if (info.commandID == clear_recent_files) {
        recent_files_->clear();
        menuItemsChanged(); // Refresh menu
        return true;
    }
}
```

### 6. Update file open/save operations to add files to recent list

**Location:** `Source/MainComponent.cpp`

**Changes needed:**
- Add RecentFiles instance as member variable in `MainComponent.h`
- Update `loadDocument()` (line 188-213) to add successfully loaded files
- Update `saveAsync()` success callback (line 233-240) to add saved files
- Update `saveAsInteractiveAsync()` callback to add newly saved files
- Only add files that were successfully loaded/saved

**Integration points:**
- `loadDocument()`: After successful load, call
  `recent_files_->addFile(file)`
- `saveAsync()`: In success callback, call `recent_files_->addFile(file)`
- `saveAsInteractiveAsync()`: In success callback, add the new file path

### 7. Handle recent files validation (check if files still exist)

**Location:** `Source/km/recent_files.cpp`

**Validation strategy:**
- Before returning list, verify each file exists using
  `juce::File::existsAsFile()`
- Remove non-existent files from the stored list
- Update settings if files were removed
- Optional: Show missing files in gray with "(missing)" suffix

**Implementation:**
```cpp
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
        if (changed) {
            save();
        }
    }
    return recent_files_;
}
```

### 8. Test recent files menu functionality

**Test scenarios:**

1. **Capacity test:** Open 8+ different files, verify only 8 are kept
2. **Duplicate handling:** Open same file twice, verify it moves to top (no
   duplicates)
3. **File deletion:** Delete a file from filesystem, verify it's removed from
   list on next access
4. **Persistence:** Close and reopen app, verify recent files persist across
   sessions
5. **Clear function:** Clear recent files, verify list is empty and menu
   updates
6. **Open from recent:** Click recent file, verify it opens correctly
7. **New project:** Verify new project doesn't add to recent (since no file
   path yet)
8. **Save as:** Verify "Save As" with new name updates recent files with new
   path
9. **Menu state:** Verify menu items are disabled when list is empty
10. **Error handling:** Try to open a recent file that no longer exists,
    verify graceful error handling

## Files to Create

- `Source/km/recent_files.h`
- `Source/km/recent_files.cpp`
- `docs/recent-files-implementation.md` (this file)

## Files to Modify

- `Source/gui/menu_manager.h` - Add RecentFiles member
- `Source/gui/menu_manager.cpp` - Add commands, menu items, handlers
- `Source/MainComponent.h` - Add RecentFiles member and methods
- `Source/MainComponent.cpp` - Update file operations to track recent files

## Settings Storage

**Key:** `km.recent_files`

**Format:** Pipe-delimited string of absolute file paths

**Example:**
```
/Users/name/Documents/project1.kmst|/Users/name/Music/show.kmst|...
```

**Maximum entries:** 8 files

**Storage location:** Same as other KeyMaster settings
- macOS: `~/Library/Application Support/KeyMaster/KeyMaster.props`
- Linux: `~/.config/KeyMaster/KeyMaster.props`
- Windows: `%APPDATA%\KeyMaster\KeyMaster.props`

## Integration with Existing Code

This feature integrates cleanly with the existing architecture:

1. **Settings system:** Uses existing `ApplicationProperties` infrastructure
   from `Main.cpp`
2. **Menu system:** Follows existing `ApplicationCommandTarget` pattern in
   `MenuManager`
3. **File operations:** Hooks into existing `loadDocument()`, `saveAsync()`,
   and `saveAsInteractiveAsync()` methods
4. **Code organization:** New `RecentFiles` class follows existing pattern in
   `Source/km/` directory

## Implementation Notes

- Follow JUCE best practices for menu handling and file I/O
- Maintain consistency with existing code style and patterns
- Consider edge cases like network drives, moved files, and renamed files
- Ensure thread safety if file operations are async
- Use JUCE's `File` class for all file path operations (cross-platform)
- Display shortened paths in menu if full paths are too long (optional
  enhancement)

## Future Enhancements (Optional)

- Add keyboard shortcuts for recent files (Cmd+1 through Cmd+8)
- Show file icons in menu
- Display relative dates ("opened today", "opened yesterday")
- Separate "Recent" from "Pinned" files
- Context menu on recent files for "Remove from list" or "Show in Finder"
