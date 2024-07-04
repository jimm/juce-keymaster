#pragma once

#include <JuceHeader.h>
#include "gui/menu_manager.h"
#include "gui/list_box_model.h"
#include "gui/patch_table.h"

class KeyMaster;

//==============================================================================
/*
  This component lives inside our window, and this is where you should put all
  your controls and content.
*/
class MainComponent : public Component
{
public:
  //==============================================================================
  MainComponent(ApplicationProperties &app_properties);
  ~MainComponent() override;

  //==============================================================================
  void paint(Graphics& g) override;
  void resized() override;

private:
  //==============================================================================

  KeyMaster *km;
  MenuManager menu_manager;

  OwnedArray<ListBoxModel> box_models;
  OwnedArray<PatchTableListBoxModel> table_box_models;

  Label set_list_songs_label;
  ListBox set_list_songs;

  Label song_patches_label;
  ListBox song_patches;

  Label song_notes_label;
  Label song_notes;

  Label set_lists_label;
  ListBox set_lists;

  Label messages_label;
  ListBox messages;

  Label triggers_label;
  ListBox triggers;

  Label patch_table_label;
  PatchTableListBoxModel *patch_table;

  void make_menu_bar();

  void config_label(Label &label, const char *text);
  void config_list_box(const char *label_text, Label &label, ListBox &list_box, ListBoxModel *model);

  void make_set_list_songs_pane();
  void make_song_patches_pane();
  void make_song_notes_pane();
  void make_set_lists_pane();
  void make_messages_pane();
  void make_triggers_pane();
  void make_patch_pane();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
