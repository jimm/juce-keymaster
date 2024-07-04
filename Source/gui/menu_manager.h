#pragma once

#include <JuceHeader.h>

class KeyMaster;

class MenuManager : public ApplicationCommandTarget, public MenuBarModel {
public:
  MenuManager() {}
  ~MenuManager() override;

  // ================ ApplicationCommandTarget

  ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }
  void getAllCommands (Array<CommandID>& c) override;
  void getCommandInfo(CommandID commandID, ApplicationCommandInfo &result) override;
  bool perform(const InvocationInfo& info) override;

  // ================ MenuBarModel

  StringArray getMenuBarNames() override;
  PopupMenu getMenuForIndex(int menuIndex, const String& _menuName) override;
  void menuItemSelected(int, int) override {}

  // ================ other

  void set_keymaster(KeyMaster *km_ptr) { km = km_ptr; }
  void make_menu_bar(Component *c);

  // May update area
  void resized(Rectangle<int> &area);

private:
  KeyMaster *km;
  std::unique_ptr<MenuBarComponent> menu_bar;
  ApplicationCommandManager command_manager;

  void warn_unimplemented();
};
