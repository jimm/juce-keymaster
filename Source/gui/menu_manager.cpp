#include "menu_manager.h"
#include "../km/keymaster.h"

enum CommandIDs {
  // File
  new_project = 1,
  open_project,
  close_project,
  save_project,
  save_project_as,
  // Edit
  undo,
  redo,
  cut,
  copy,
  paste,
  new_message,
  new_trigger,
  new_song,
  new_patch,
  new_connection,
  new_set_list,
  delete_message,
  delete_trigger,
  delete_song,
  delete_patch,
  delete_connection,
  delete_set_list,
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

MenuManager::~MenuManager() {
#if JUCE_MAC
  MenuBarModel::setMacMainMenu(nullptr);
#endif
  command_manager.setFirstCommandTarget(nullptr);
}

// ================ ApplicationCommandTarget


void MenuManager::getAllCommands (Array<CommandID>& c) {
  Array<CommandID> commands {
    // File
    CommandIDs::new_project,
    CommandIDs::open_project,
    CommandIDs::close_project,
    CommandIDs::save_project,
    CommandIDs::save_project_as,
    // Edit
    CommandIDs::undo,
    CommandIDs::redo,
    CommandIDs::cut,
    CommandIDs::copy,
    CommandIDs::paste,
    CommandIDs::new_message,
    CommandIDs::new_trigger,
    CommandIDs::new_song,
    CommandIDs::new_patch,
    CommandIDs::new_connection,
    CommandIDs::new_set_list,
    CommandIDs::delete_message,
    CommandIDs::delete_trigger,
    CommandIDs::delete_song,
    CommandIDs::delete_patch,
    CommandIDs::delete_connection,
    CommandIDs::delete_set_list,
    // Go
    CommandIDs::next_song,
    CommandIDs::prev_song,
    CommandIDs::next_patch,
    CommandIDs::prev_patch,
    CommandIDs::find_song,
    CommandIDs::find_set_list,
    // MIDI
    CommandIDs::toggle_clock,
    CommandIDs::all_notes_off,
    CommandIDs::super_panic,
    CommandIDs::midi_monitor
  };
  c.addArray(commands);
}

void MenuManager::getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) {
  switch (commandID) {
  // ==== File
  case CommandIDs::new_project:
    result.setInfo("New Project", "Creates a new project", "File", 0);
    result.addDefaultKeypress('n', ModifierKeys::commandModifier);
    break;
  case CommandIDs::open_project:
    result.setInfo("Open Project", "Opens an existing project", "File", 0);
    result.addDefaultKeypress('o', ModifierKeys::commandModifier);
    break;
  case CommandIDs::close_project:
    result.setInfo("Close Project", "Closes the project", "File", 0);
    break;
  case CommandIDs::save_project:
    result.setInfo("Save", "Saves the project", "File", 0);
    result.addDefaultKeypress('s', ModifierKeys::commandModifier);
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
    result.setInfo("New Trigger", "Quits KeyMaster", "Edit", 0);
    result.addDefaultKeypress('t', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_song:
    result.setInfo("New Song", "Quits KeyMaster", "Edit", 0);
    result.addDefaultKeypress('g', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_patch:
    result.setInfo("New Patch", "Quits KeyMaster", "Edit", 0);
    result.addDefaultKeypress('h', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_connection:
    result.setInfo("New Connection", "Quits KeyMaster", "Edit", 0);
    result.addDefaultKeypress('e', ModifierKeys::commandModifier);
    break;
  case CommandIDs::new_set_list:
    result.setInfo("New Set List", "Quits KeyMaster", "Edit", 0);
    result.addDefaultKeypress('l', ModifierKeys::commandModifier);
    break;
  case CommandIDs::delete_message:
    result.setInfo("Delete Message", "Quits KeyMaster", "Edit", 0);
    break;
  case CommandIDs::delete_trigger:
    result.setInfo("Delete Trigger", "Quits KeyMaster", "Edit", 0);
    break;
  case CommandIDs::delete_song:
    result.setInfo("Delete Song", "Quits KeyMaster", "Edit", 0);
    break;
  case CommandIDs::delete_patch:
    result.setInfo("Delete Patch", "Quits KeyMaster", "Edit", 0);
    break;
  case CommandIDs::delete_connection:
    result.setInfo("Delete Connection", "Quits KeyMaster", "Edit", 0);
    break;
  case CommandIDs::delete_set_list:
    result.setInfo("Delete Set List", "Quits KeyMaster", "Edit", 0);
    break;
  // ==== Go
  case CommandIDs::next_song:
    result.setInfo("Next Song", "Quits KeyMaster", "Go", 0);
    break;
  case CommandIDs::prev_song:
    result.setInfo("Prev Song", "Quits KeyMaster", "Go", 0);
    break;
  case CommandIDs::next_patch:
    result.setInfo("Next Patch", "Quits KeyMaster", "Go", 0);
    break;
  case CommandIDs::prev_patch:
    result.setInfo("Prev Patch", "Quits KeyMaster", "Go", 0);
    break;
  case CommandIDs::find_song:
    result.setInfo("Find a Song", "Quits KeyMaster", "Go", 0);
    break;
  case CommandIDs::find_set_list:
    result.setInfo("Find a Set List", "Quits KeyMaster", "Go", 0);
    break;
  // ==== MIDI
  case CommandIDs::toggle_clock:
    result.setInfo("Toggle MIDI Clock", "Quits KeyMaster", "MIDI", 0);
    result.addDefaultKeypress('k', ModifierKeys::commandModifier);
    break;
  case CommandIDs::all_notes_off:
    result.setInfo("All Notes Off", "Quits KeyMaster", "MIDI", 0);
    result.addDefaultKeypress('a', ModifierKeys::commandModifier);
    break;
  case CommandIDs::super_panic:
    result.setInfo("Super Panic!", "Quits KeyMaster", "MIDI", 0);
    result.addDefaultKeypress('q', ModifierKeys::commandModifier);
    break;
  case CommandIDs::midi_monitor:
    result.setInfo("Midi Monitor", "Quits KeyMaster", "MIDI", 0);
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
    warn_unimplemented();
    break;
  case CommandIDs::open_project:
    break;
  case CommandIDs::close_project:
    break;
  case CommandIDs::save_project:
    break;
  case CommandIDs::save_project_as:
    break;
  // ==== Edit
  case CommandIDs::undo:
    break;
  case CommandIDs::redo:
    break;
  case CommandIDs::cut:
    break;
  case CommandIDs::copy:
    break;
  case CommandIDs::paste:
    break;
  case CommandIDs::new_message:
    break;
  case CommandIDs::new_trigger:
    break;
  case CommandIDs::new_song:
    break;
  case CommandIDs::new_patch:
    break;
  case CommandIDs::new_connection:
    break;
  case CommandIDs::new_set_list:
    break;
  case CommandIDs::delete_message:
    break;
  case CommandIDs::delete_trigger:
    break;
  case CommandIDs::delete_song:
    break;
  case CommandIDs::delete_patch:
    break;
  case CommandIDs::delete_connection:
    break;
  case CommandIDs::delete_set_list:
    break;
  // ==== Go
  case CommandIDs::next_song:
    break;
  case CommandIDs::prev_song:
    break;
  case CommandIDs::next_patch:
    break;
  case CommandIDs::prev_patch:
    break;
  case CommandIDs::find_song:
    break;
  case CommandIDs::find_set_list:
    break;
  // ==== MIDI
  case CommandIDs::toggle_clock:
    break;
  case CommandIDs::all_notes_off:
    break;
  case CommandIDs::super_panic:
    break;
  case CommandIDs::midi_monitor:
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

  switch (menuIndex) {
  case 0:                       // File
    menu.addCommandItem(&command_manager, CommandIDs::new_project);
    menu.addCommandItem(&command_manager, CommandIDs::open_project);
    menu.addCommandItem(&command_manager, CommandIDs::close_project);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::save_project);
    menu.addCommandItem(&command_manager, CommandIDs::save_project_as);
    break;
  case 1:                       // Edit
    menu.addCommandItem(&command_manager, CommandIDs::undo);
    menu.addCommandItem(&command_manager, CommandIDs::redo);
    menu.addCommandItem(&command_manager, CommandIDs::cut);
    menu.addCommandItem(&command_manager, CommandIDs::copy);
    menu.addCommandItem(&command_manager, CommandIDs::paste);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::new_message);
    menu.addCommandItem(&command_manager, CommandIDs::new_trigger);
    menu.addCommandItem(&command_manager, CommandIDs::new_song);
    menu.addCommandItem(&command_manager, CommandIDs::new_patch);
    menu.addCommandItem(&command_manager, CommandIDs::new_connection);
    menu.addCommandItem(&command_manager, CommandIDs::new_set_list);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::delete_message);
    menu.addCommandItem(&command_manager, CommandIDs::delete_trigger);
    menu.addCommandItem(&command_manager, CommandIDs::delete_song);
    menu.addCommandItem(&command_manager, CommandIDs::delete_patch);
    menu.addCommandItem(&command_manager, CommandIDs::delete_connection);
    menu.addCommandItem(&command_manager, CommandIDs::delete_set_list);
    break;
  case 2:                       // Go
    menu.addCommandItem(&command_manager, CommandIDs::next_song);
    menu.addCommandItem(&command_manager, CommandIDs::prev_song);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::next_patch);
    menu.addCommandItem(&command_manager, CommandIDs::prev_patch);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::find_song);
    menu.addCommandItem(&command_manager, CommandIDs::find_set_list);
    break;
  case 3:                       // MIDI
    menu.addCommandItem(&command_manager, CommandIDs::toggle_clock);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::all_notes_off);
    menu.addCommandItem(&command_manager, CommandIDs::super_panic);
    menu.addSeparator();
    menu.addCommandItem(&command_manager, CommandIDs::midi_monitor);
    break;
  }
  return menu;
}

// ================ other

void MenuManager::make_menu_bar(Component *c) {
#if JUCE_MAC
  MenuBarModel::setMacMainMenu(this);
#else
  menu_bar.reset(new MenuBarComponent(this));
  c->addAndMakeVisible(menu_bar.get());
#endif

  setApplicationCommandManagerToWatch(&command_manager);
  command_manager.registerAllCommandsForTarget(this);

  // this ensures that commands invoked on the application are correctly
  // forwarded to this demo
  command_manager.setFirstCommandTarget(this);

  // this lets the command manager use keypresses that arrive in our window to send out commands
  c->addKeyListener(command_manager.getKeyMappings());

  c->setWantsKeyboardFocus(true);
}

void MenuManager::resized(Rectangle<int> &area) {
#ifndef JUCE_MAC
  menu_bar->setBounds(area.removeFromTop(LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
#endif
}

void MenuManager::warn_unimplemented() {
  AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon, "Not Implemented",
                                   "This command has not yet been implemented");
}
