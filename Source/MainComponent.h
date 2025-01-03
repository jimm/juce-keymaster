#pragma once

#include <JuceHeader.h>
#include "km/device_manager.h"
#include "gui/clock_bpm.h"
#include "gui/clock_button.h"
#include "gui/clock_light.h"
#include "gui/connections_table.h"
#include "gui/menu_manager.h"
#include "gui/messages_list_box.h"
#include "gui/set_list_songs_list_box.h"
#include "gui/set_lists_list_box.h"
#include "gui/song_notes.h"
#include "gui/song_patches_list_box.h"
#include "gui/triggers_table.h"
#include "gui/midi_monitor.h"

class KeyMaster;

//==============================================================================
/*
  This component lives inside our window, and this is where you should put all
  your controls and content.
*/
class MainComponent : public Component, public FileBasedDocument, public ActionListener
{
public:
  //==============================================================================
  MainComponent(DeviceManager &device_manager, ApplicationProperties &app_properties);
  ~MainComponent() override;

  //==============================================================================
  void paint(Graphics& g) override;
  void resized() override;
  void update();

  //==============================================================================
  void new_project();
  void open_project();
  void save_project();
  void save_project_as();
  void undo();
  void redo();
  void cut();
  void copy();
  void paste();
  void new_message();
  void new_trigger();
  void new_song();
  void new_patch();
  void new_connection();
  void new_set_list();
  void delete_message();
  void delete_trigger();
  void delete_song();
  void delete_patch();
  void delete_connection();
  void delete_set_list();
  void next_song();
  void prev_song();
  void next_patch();
  void prev_patch();
  void find_song();
  void find_set_list();
  void toggle_clock();
  void all_notes_off();
  void super_panic();
  void midi_monitor();

  // ================ loading and saving ================
  void check_ok_to_quit(std::function<void (bool)> callback);
  String getDocumentTitle() override { return "KeyMaster"; }
  Result loadDocument(const File &file) override;
  Result saveDocument(const File &file) override;
  File getLastDocumentOpened() override { return _file; }
  void setLastDocumentOpened(const File &file) override { _file = file; }

  void actionListenerCallback(const String &message) override;

private:
  //==============================================================================
  DeviceManager &device_manager;
  ApplicationProperties &app_properties;
  File _file;
  std::unique_ptr<MidiMonitorWindow> _midi_monitor_window;

  MenuManager menu_manager;

  OwnedArray<ListBoxModel> box_models;
  OwnedArray<TableListBoxModel> table_box_models;

  Label set_list_songs_label;
  SetListSongsListBox set_list_songs;

  Label song_patches_label;
  SongPatchesListBox song_patches;

  Label clock_label;
  ClockLight clock_light;
  ClockBpm bpm;
  Label bpm_label { {}, "BPM" };
  ClockButton clock_button;

  Label song_notes_label;
  SongNotes song_notes;

  Label connections_table_label;
  ConnectionsTableListBox connections_table;

  Label set_lists_label;
  SetListsListBox set_lists;

  Label messages_label;
  MessagesListBox messages;

  Label triggers_label;
  TriggersTableListBox triggers;

  void load_or_create_keymaster();
  void create_new_project();

  void make_menu_bar();

  void config_label(Label &label, const char *text);
  void config_lbox(const char *label_text, Label &label, ListBox &list_box);
  void config_list_box(
    const char *label_text, Label &label, KmListBox &list_box, KmListBoxModel *model);
  void config_table_list_box(
    const char *label_text, Label &label, KmTableListBox &list_box, KmTableListBoxModel *model);

  void update_clock_contents();

  void layout_top(Rectangle<int> &area);
  void layout_set_list_songs_and_song_patches(Rectangle<int> &area);
  void layout_clock_and_notes(Rectangle<int> &area);
  void layout_middle(Rectangle<int> &area);
  void layout_bottom(Rectangle<int> &area);

  void init_song_notes();

  void make_set_list_songs_pane();
  void make_song_patches_pane();
  void make_clock_pane();
  void make_song_notes_pane();
  void make_connections_pane();
  void make_set_lists_pane();
  void make_messages_pane();
  void make_triggers_pane();

  void add_km_listeners();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
