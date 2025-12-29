# KeyMaster Codebase Structure

This document provides an overview of the KeyMaster application architecture
for AI assistance and future development reference.

## Application Overview

KeyMaster is a JUCE-based MIDI router and processor that manages MIDI
connections, songs, patches, and real-time MIDI routing. It uses a
document-based architecture with JSON file storage.

## Architecture Pattern

```
KeyMasterApplication (Main.cpp)
  └─ MainWindow
       └─ MainComponent (FileBasedDocument)
            ├─ MenuManager (ApplicationCommandTarget)
            ├─ SetListSongsListBox
            ├─ SongPatchesListBox
            ├─ ConnectionsTable
            ├─ SetListsListBox
            ├─ MessagesListBox
            ├─ TriggersTable
            ├─ Clock UI (Clock light, BPM, button)
            └─ SongNotes
```

## Key Components

### 1. Menu System

**Files:**
- `Source/gui/menu_manager.h` (38 lines)
- `Source/gui/menu_manager.cpp` (457 lines)

**Architecture:**
Uses JUCE's `ApplicationCommandTarget` and `MenuBarModel` pattern.

**Command Categories:**
- **File:** new_project, open_project, save_project, save_project_as
- **Edit:** undo, redo, cut, copy, paste, new_song, new_patch,
  new_connection, new_set_list, new_message, new_trigger, delete operations
- **Go:** next_song, prev_song, next_patch, prev_patch, find_song,
  find_set_list
- **MIDI:** toggle_clock, all_notes_off, super_panic, midi_monitor
- **Help:** about (macOS only)

**Command Flow:**
1. User clicks menu item
2. MenuManager routes command via `perform()` method (lines 288-396)
3. Delegates to MainComponent methods
4. Changes trigger ActionBroadcaster notifications
5. UI components update via ActionListener callbacks

### 2. Settings & Preferences

**Files:**
- `Source/Main.cpp` (153 lines) - Application lifecycle and settings init
- `Source/MainComponent.h` (154 lines)
- `Source/MainComponent.cpp` (558 lines)

**Storage Mechanism:**
Uses JUCE's `ApplicationProperties` with XML format.

**Initialization:** `Main.cpp` lines 24-35
- Application name: "KeyMaster"
- File suffix: ".props"
- macOS location: `~/Library/Application Support/KeyMaster/`

**Current Settings Keys:**
- `"window.state"` - Window position and size
- `"window.width"` - Window width
- `"window.height"` - Window height
- `"km.file"` - Last opened project file path (constant at line 20)

**Shutdown:** `Main.cpp` lines 40-50
- Saves window state on exit
- Closes property files to flush to disk

### 3. Project File Operations

**Files:**
- `Source/km/storage.h` (61 lines)
- `Source/km/storage.cpp` (555 lines)
- `Source/MainComponent.cpp` (558 lines)

**File Format:**
- Extension: `.kmst`
- Format: JSON (using JUCE's JSON parser)

**Key Operations:**

**Initial Load** (`MainComponent.cpp` lines 160-186):
- Checks if `"km.file"` property exists
- Verifies file still exists
- Calls `loadDocument()` if valid
- Falls back to new project if file missing

**New Project** (lines 215-220):
- Prompts to save current project first (async)
- Creates new KeyMaster instance
- Removes old file from settings

**Open Project** (lines 222-231):
- Prompts to save current project first
- Opens file browser (via JUCE's loadFromUserSpecifiedFileAsync)
- Calls loadDocument with selected file

**Save Project** (lines 233-240):
- Calls saveAsync with async callback
- Updates settings with file path on success

**Save As** (lines 242-244):
- Calls saveAsInteractiveAsync
- Opens file dialog for new location

**Storage Class** (`storage.cpp`):
- `load()` method: Parses JSON, reconstructs KeyMaster object hierarchy
- `save()` method: Serializes KeyMaster to JSON, writes to file
- Returns error strings for UI display

### 4. Core Data Model

**Location:** `Source/km/`

**Main Classes:**
- `keymaster.h/cpp` - Main application state holder, root of data hierarchy
- `cursor.h/cpp` - Navigation state (set_list, song, patch, connection,
  message, trigger indices)
- `set_list.h/cpp` - Contains array of Song objects
- `song.h/cpp` - Contains patches, notes, BPM, clock settings
- `patch.h/cpp` - Container for connections
- `connection.h/cpp` - Maps MIDI input to output with filtering/transformation
- `trigger.h/cpp` - Custom keyboard/MIDI action bindings
- `message_block.h/cpp` - MIDI message definitions
- `curve.h/cpp` - CC value transformation curves
- `device_manager.h/cpp` - MIDI I/O device management

**Data Hierarchy:**
```
KeyMaster
  ├─ Cursor (current selections)
  ├─ SetList[]
  │    └─ Song[]
  │         └─ Patch[]
  │              └─ Connection[]
  ├─ MessageBlock[]
  ├─ Trigger[]
  └─ DeviceManager
```

### 5. GUI Components

**Location:** `Source/gui/`

**Base Classes:**
- `KmEditor` - Dialog-based editors with Apply/Ok/Cancel buttons

**Specific Editors:**
- `SongEditor` - Edit song properties
- `PatchEditor` - Edit patch properties
- `ConnectionEditor` - Edit MIDI connection routing
- `SetListEditor` - Edit set list properties

**List Components:**
- `SetListSongsListBox` - Songs in current set list
- `SongPatchesListBox` - Patches in current song
- `ConnectionsTable` - Connections in current patch
- `SetListsListBox` - All set lists
- `MessagesListBox` - Message blocks
- `TriggersTable` - Trigger definitions

**Special Components:**
- `MidiMonitor` - Separate window for MIDI message monitoring
- `ClockLight` - Visual MIDI clock indicator
- `ClockBpm` - BPM display and editor
- `ClockButton` - Start/stop clock button

### 6. Interaction Flow

1. User interacts with GUI (menu, button, list box)
2. Component calls MainComponent method or KeyMaster method
3. Data model updates
4. Model calls `KeyMaster::changed()` to mark document dirty
5. Model sends ActionBroadcaster notification
6. UI components receive ActionListener callbacks
7. Components refresh their display

## File Organization

```
Source/
├── Main.cpp                  # Application entry point, settings init
├── MainComponent.h/cpp       # Main window, document management
├── km/                       # Core data model
│   ├── keymaster.h/cpp       # Root data object
│   ├── cursor.h/cpp          # Navigation state
│   ├── set_list.h/cpp        # Set list container
│   ├── song.h/cpp            # Song with patches
│   ├── patch.h/cpp           # Patch with connections
│   ├── connection.h/cpp      # MIDI routing
│   ├── message_block.h/cpp   # MIDI messages
│   ├── trigger.h/cpp         # Key/MIDI bindings
│   ├── storage.h/cpp         # JSON serialization
│   └── device_manager.h/cpp  # MIDI devices
└── gui/                      # UI components
    ├── menu_manager.h/cpp    # Application menus
    ├── *_editor.h/cpp        # Dialog editors
    ├── *_list_box.h/cpp      # List components
    └── clock_*.h/cpp         # Clock UI components
```

## Important Patterns

### Command Pattern
All menu operations use JUCE's ApplicationCommandTarget pattern:
- Commands defined in enum
- `getAllCommands()` returns available command IDs
- `getCommandInfo()` provides command metadata
- `perform()` executes commands

### Document Pattern
MainComponent extends JUCE's FileBasedDocument:
- Tracks dirty state
- Handles save confirmations
- Manages file associations

### Observer Pattern
Model changes broadcast via JUCE's ActionBroadcaster/ActionListener:
- Data objects inherit ActionBroadcaster
- UI components implement ActionListener
- Automatic UI updates on data changes

## Development Guidelines

### File Operations
- Always use JUCE's `File` class for cross-platform compatibility
- File operations are async (use callbacks)
- Validate file existence before operations
- Update settings after successful file operations

### Menu Development
- Add commands to CommandIDs enum
- Register in getAllCommands()
- Add info in getCommandInfo()
- Implement in perform()
- Delegate to MainComponent methods

### Settings Management
- Use ApplicationProperties for persistence
- Define string constants for keys
- Save on successful operations
- Load on startup with validation

### UI Updates
- Inherit ActionBroadcaster for data models
- Implement ActionListener for UI components
- Use `actionListenerCallback()` to refresh UI
- Keep UI logic separate from business logic

## Build System

KeyMaster uses JUCE's Projucer for project management. The project files
are not checked into git; instead, developers generate them from the
`.jucer` file.

## Testing Considerations

When implementing new features:
1. Test cross-platform file path handling
2. Verify settings persistence across restarts
3. Test with missing/moved files
4. Verify undo/redo compatibility
5. Check memory management with JUCE smart pointers
6. Test async operations and callbacks

## Common Integration Points

When adding new features, you'll likely interact with:
- **MenuManager**: Add new menu items and commands
- **MainComponent**: Add document-level operations
- **ApplicationProperties**: Store user preferences
- **KeyMaster**: Access/modify core data model
- **Storage**: Serialize new data to/from JSON
