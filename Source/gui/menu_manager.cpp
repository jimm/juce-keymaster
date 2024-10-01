#include "menu_manager.h"
#include "../MainComponent.h"
#include "../km/keymaster.h"

enum CommandIDs {
  __separator__ = 1,
  // File
  new_project,
  open_project,
  save_project,
  save_project_as,
  // Edit
  undo,
  redo,
  cut,
  copy,
  paste,
  new_song,
  new_patch,
  new_connection,
  new_set_list,
  new_message,
  new_trigger,
  delete_song,
  delete_patch,
  delete_connection,
  delete_set_list,
  delete_message,
  delete_trigger,
  // Go
  next_song,
  prev_song,
  next_patch,
  prev_patch,
  find_song,
  find_set_list,
  // MIDI
  toggle_clock,
  all_notes_off,
  super_panic,
  midi_monitor
};

static Array<CommandIDs> file_commands {
  CommandIDs::new_project,
  CommandIDs::open_project,
  CommandIDs::__separator__,
  CommandIDs::save_project,
  CommandIDs::save_project_as,
};
static Array<CommandIDs> edit_commands {
  CommandIDs::undo,
  CommandIDs::redo,
  CommandIDs::cut,
  CommandIDs::copy,
  CommandIDs::paste,
  CommandIDs::__separator__,
  CommandIDs::new_message,
  CommandIDs::new_trigger,
  CommandIDs::new_song,
  CommandIDs::new_patch,
  CommandIDs::new_connection,
  CommandIDs::new_set_list,
  CommandIDs::__separator__,
  CommandIDs::delete_message,
  CommandIDs::delete_trigger,
  CommandIDs::delete_song,
  CommandIDs::delete_patch,
  CommandIDs::delete_connection,
  CommandIDs::delete_set_list,
};
static Array<CommandIDs> go_commands {
  CommandIDs::next_song,
  CommandIDs::prev_song,
  CommandIDs::__separator__,
  CommandIDs::next_patch,
  CommandIDs::prev_patch,
  CommandIDs::__separator__,
  CommandIDs::find_song,
  CommandIDs::find_set_list,
};
static Array<CommandIDs> midi_commands {
  CommandIDs::toggle_clock,
  CommandIDs::__separator__,
  CommandIDs::all_notes_off,
  CommandIDs::super_panic,
  CommandIDs::__separator__,
  CommandIDs::midi_monitor
};
static Array<CommandIDs> *command_arrays[4] = {
  &file_commands,
  &edit_commands,
  &go_commands,
  &midi_commands,
};

MenuManager::~MenuManager() {
#if JUCE_MAC
  MenuBarModel::setMacMainMenu(nullptr);
#endif
  command_manager.setFirstCommandTarget(nullptr);
}

// ================ ApplicationCommandTarget

void MenuManager::getAllCommands (Array<CommandID>& c) {
  c.addArray(file_commands);
  c.addArray(edit_commands);
  c.addArray(go_commands);
  c.addArray(midi_commands);
  c.removeAllInstancesOf(CommandIDs::__separator__);
}

void MenuManager::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) {
  KeyMaster *km = KeyMaster_instance();
  Song *song;
  Patch *patch;

  switch (commandID) {
  // ==== File
  case CommandIDs::new_project:
    result.setInfo("New Project", "Creates a new project", "File", 0);
    result.addDefaultKeypress('n', ModifierKeys::commandModifier);
    break;
  case CommandIDs::open_project:
    result.setInfo("Open Project",
                   "Opens an existing project (closes open one)", "File", 0);
    result.addDefaultKeypress('o', ModifierKeys::commandModifier);
    break;
  case CommandIDs::save_project:
    result.setInfo("Save", "Saves the project", "File", 0);
    result.addDefaultKeypress('s', ModifierKeys::commandModifier);
    // TODO only active if km is modified
    break;
  case CommandIDs::save_project_as:
    result.setInfo("Save As...", "Saves the project into a different file", "File", 0);
    break;
  // ==== Edit
  case CommandIDs::undo:
    result.setInfo("Undo", "Undoes the previous action", "Edit", 0);
    result.addDefaultKeypress('z', ModifierKeys::commandModifier);
    break;
  case CommandIDs::redo:
    result.setInfo("Redo", "Redoes the previous undo", "Edit", 0);
    result.addDefaultKeypress('r', ModifierKeys::commandModifier);
    break;
  case CommandIDs::cut:
    result.setInfo("Cut", "Cut", "Edit", 0);
    result.addDefaultKeypress('x', ModifierKeys::commandModifier);
    break;
  case CommandIDs::copy:
    result.setInfo("Copy", "Copy", "Edit", 0);
    result.addDefaultKeypress('c', ModifierKeys::commandModifier);
    break;
  case CommandIDs::paste:
    result.setInfo("Paste", "Paste", "Edit", 0);
    result.addDefaultKeypress('v', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_message:
    result.setInfo("New Message", "Creates a new message", "Edit", 0);
    result.addDefaultKeypress('m', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_trigger:
    result.setInfo("New Trigger", "New Trigger", "Edit", 0);
    result.addDefaultKeypress('t', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_song:
    result.setInfo("New Song", "New Song", "Edit", 0);
    result.addDefaultKeypress('g', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_patch:
    result.setInfo("New Patch", "New Patch", "Edit", 0);
    result.addDefaultKeypress('a', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_connection:
    result.setInfo("New Connection", "New Connection", "Edit", 0);
    result.addDefaultKeypress('e', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_set_list:
    result.setInfo("New Set List", "New Set List", "Edit", 0);
    result.addDefaultKeypress('l', ModifierKeys::commandModifier);
    break;
  case CommandIDs::delete_message:
    result.setInfo("Delete Message", "Delete Message", "Edit", 0);
    result.setActive(km->messages().size() > 0);
    break;
  case CommandIDs::delete_trigger:
    result.setInfo("Delete Trigger", "Delete Trigger", "Edit", 0);
    result.setActive(km->triggers().size() > 0);
    break;
  case CommandIDs::delete_song:
    result.setInfo("Delete Song", "Delete Song", "Edit", 0);
    result.setActive(km->all_songs()->songs().size() > 1);
    break;
  case CommandIDs::delete_patch:
    result.setInfo("Delete Patch", "Delete Patch", "Edit", 0);
    song = km->cursor()->song();
    result.setActive(song != nullptr && song->patches().size() > 0);
    break;
  case CommandIDs::delete_connection:
    result.setInfo("Delete Connection", "Delete Connection", "Edit", 0);
    patch = km->cursor()->patch();
    result.setActive(patch != nullptr && patch->connections().size() > 0);
    break;
  case CommandIDs::delete_set_list:
    result.setInfo("Delete Set List", "Delete Set List", "Edit", 0);
    result.setActive(km->cursor()->set_list() != km->all_songs());
    break;
  // ==== Go
  case CommandIDs::next_song:
    result.setInfo("Next Song", "Next Song", "Go", 0);
    result.addDefaultKeypress('n', 0);
    result.addDefaultKeypress(KeyPress::rightKey, 0);
    result.setActive(km->cursor()->has_next_song());
    break;
  case CommandIDs::prev_song:
    result.setInfo("Prev Song", "Prev Song", "Go", 0);
    result.addDefaultKeypress('p', 0);
    result.addDefaultKeypress(KeyPress::leftKey, 0);
    result.setActive(km->cursor()->has_prev_song());
    break;
  case CommandIDs::next_patch:
    result.setInfo("Next Patch", "Next Patch", "Go", 0);
    result.addDefaultKeypress('j', 0);
    result.addDefaultKeypress(KeyPress::downKey, 0);
    result.setActive(km->cursor()->has_next_patch());
    break;
  case CommandIDs::prev_patch:
    result.setInfo("Prev Patch", "Prev Patch", "Go", 0);
    result.addDefaultKeypress('k', 0);
    result.addDefaultKeypress(KeyPress::upKey, 0);
    result.setActive(km->cursor()->has_prev_patch());
    break;
  case CommandIDs::find_song:
    result.setInfo("Find a Song", "Find a Song", "Go", 0);
    result.addDefaultKeypress('f', ModifierKeys::commandModifier);
    break;
  case CommandIDs::find_set_list:
    result.setInfo("Find a Set List", "Find a Set List", "Go", 0);
    result.addDefaultKeypress('t', ModifierKeys::commandModifier);
    break;
  // ==== MIDI
  case CommandIDs::toggle_clock:
    result.setInfo("Toggle MIDI Clock", "Toggle MIDI Clock", "MIDI", 0);
    result.addDefaultKeypress('k', ModifierKeys::commandModifier);
    break;
  case CommandIDs::all_notes_off:
    result.setInfo("All Notes Off", "All Notes Off", "MIDI", 0);
    result.addDefaultKeypress(KeyPress::F1Key, 0);
    break;
  case CommandIDs::super_panic:
    result.setInfo("Super Panic!", "Super Panic!", "MIDI", 0);
    result.addDefaultKeypress(KeyPress::F2Key, 0);
    break;
  case CommandIDs::midi_monitor:
    result.setInfo("Midi Monitor", "Midi Monitor", "MIDI", 0);
    result.addDefaultKeypress('i', ModifierKeys::commandModifier);
    break;
  default:
    // error
    break;
  }
}

bool MenuManager::perform(const InvocationInfo& info) {
  switch (info.commandID) {
  // ==== File
  case CommandIDs::new_project:
    handler->new_project();
    break;
  case CommandIDs::open_project:
    handler->open_project();
    break;
  case CommandIDs::save_project:
    handler->save_project();
    break;
  case CommandIDs::save_project_as:
    handler->save_project_as();
    break;
  // ==== Edit
  case CommandIDs::undo:
    handler->undo();
    break;
  case CommandIDs::redo:
    handler->redo();
    break;
  case CommandIDs::cut:
    handler->cut();
    break;
  case CommandIDs::copy:
    handler->copy();
    break;
  case CommandIDs::paste:
    handler->paste();
    break;
  case CommandIDs::new_message:
    handler->new_message();
    break;
  case CommandIDs::new_trigger:
    handler->new_trigger();
    break;
  case CommandIDs::new_song:
    handler->new_song();
    break;
  case CommandIDs::new_patch:
    handler->new_patch();
    break;
  case CommandIDs::new_connection:
    handler->new_connection();
    break;
  case CommandIDs::new_set_list:
    handler->new_set_list();
    break;
  case CommandIDs::delete_message:
    handler->delete_message();
    break;
  case CommandIDs::delete_trigger:
    handler->delete_trigger();
    break;
  case CommandIDs::delete_song:
    handler->delete_song();
    break;
  case CommandIDs::delete_patch:
    handler->delete_patch();
    break;
  case CommandIDs::delete_connection:
    handler->delete_connection();
    break;
  case CommandIDs::delete_set_list:
    handler->delete_set_list();
    break;
  // ==== Go
  case CommandIDs::next_song:
    handler->next_song();
    break;
  case CommandIDs::prev_song:
    handler->prev_song();
    break;
  case CommandIDs::next_patch:
    handler->next_patch();
    break;
  case CommandIDs::prev_patch:
    handler->prev_patch();
    break;
  case CommandIDs::find_song:
    handler->find_song();
    break;
  case CommandIDs::find_set_list:
    handler->find_set_list();
    break;
  // ==== MIDI
  case CommandIDs::toggle_clock:
    handler->toggle_clock();
    break;
  case CommandIDs::all_notes_off:
    handler->all_notes_off();
    break;
  case CommandIDs::super_panic:
    handler->super_panic();
    break;
  case CommandIDs::midi_monitor:
    handler->midi_monitor();
    break;
  default:
    return false;
  }
  return true;
}

// ================ MenuBarModel

StringArray MenuManager::getMenuBarNames() {
  return { "File", "Edit", "Go", "MIDI" };
}

PopupMenu MenuManager::getMenuForIndex(int menuIndex, const String& _menuName) {
  PopupMenu menu;
  for (auto cmd : *command_arrays[menuIndex]) {
    if (cmd == CommandIDs::__separator__)
      menu.addSeparator();
    else
      menu.addCommandItem(&command_manager, cmd);
  }
  return menu;
}

// ================ other

void MenuManager::make_menu_bar(MainComponent *c) {
  handler = c;

#if JUCE_MAC
  MenuBarModel::setMacMainMenu(this);
#else
  menu_bar.reset(new MenuBarComponent(this));
  c->addAndMakeVisible(menu_bar.get());
#endif

  setApplicationCommandManagerToWatch(&command_manager);
  command_manager.registerAllCommandsForTarget(this);

  // This ensures that commands invoked on the application are correctly
  // forwarded.
  command_manager.setFirstCommandTarget(this);
}

void MenuManager::resized(Rectangle<int> &area) {
#ifndef JUCE_MAC
  menu_bar->setBounds(area.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif
}
