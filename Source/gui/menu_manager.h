#pragma once

#include <JuceHeader.h>

class MainComponent;

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

  void make_menu_bar(MainComponent *c);

  // May update area
  void resized(Rectangle<int> &area);

private:
  MainComponent *handler;
  std::unique_ptr<MenuBarComponent> menu_bar;
  ApplicationCommandManager command_manager;

  void display_about_dialog();
};
