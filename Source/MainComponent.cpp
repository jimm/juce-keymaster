#include <JuceHeader.h>
#include "MainComponent.h"
#include "km/editor.h"
#include "km/keymaster.h"
#include "km/storage.h"
#include "gui/connection_editor.h"
#include "gui/set_list_editor.h"
#include "gui/song_editor.h"
#include "gui/midi_monitor.h"

#define SMALL_SPACE 4
#define SPACE 12
#define MAIN_LABEL_HEIGHT 20
#define DEFAULT_WINDOW_WIDTH 900
#define DEFAULT_WINDOW_HEIGHT 700
#define CLOCK_INFO_HEIGHT 24
#define CLOCK_LIGHT_WIDTH 16
#define CLOCK_BUTTON_WIDTH 80
#define BPM_WIDTH 60
#define BPM_LABEL_WIDTH 40
static const char * const KM_FILE_PROPS_KEY = "km.file";

using Track = Grid::TrackInfo;
using Fr = Grid::Fr;
using Px = Grid::Px;

MainComponent::MainComponent(DeviceManager &dev_mgr, ApplicationProperties &props)
  : FileBasedDocument(".kmst", "*.kmst", "Open project", "Save project"),
    device_manager(dev_mgr), app_properties(props)
{
  load_or_create_keymaster();   // must be before menu bar creation

  make_menu_bar();
  make_set_list_songs_pane();
  make_song_patches_pane();
  make_clock_pane();
  make_song_notes_pane();
  make_set_lists_pane();
  make_messages_pane();
  make_triggers_pane();
  make_connections_pane();

  add_km_listeners();

  auto settings = props.getUserSettings();
  auto width = settings->getIntValue("window.width", DEFAULT_WINDOW_WIDTH);
  auto height = settings->getIntValue("window.height", DEFAULT_WINDOW_HEIGHT);
  setSize(width, height);
}

MainComponent::~MainComponent()
{
  // TODO save monitor window to app settings
  // see ResizableWindow::getWindowStateAsString() and restoreWindowStateFromString()

  if (KeyMaster_instance()) {
    KeyMaster_instance()->cursor()->removeAllActionListeners();
    delete KeyMaster_instance();
  }

}

void MainComponent::paint(Graphics& g)
{
  g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
  auto area = getLocalBounds();
  menu_manager.resized(area);

  area = area.reduced(4);
  auto top_area = area.removeFromTop(area.getHeight() / 2);
  auto bottom_area = area.removeFromBottom(area.getHeight() / 2);
  auto middle_area = area;

  layout_top(top_area);
  layout_middle(middle_area);
  layout_bottom(bottom_area);
}

void MainComponent::layout_top(Rectangle<int> &area)
{
  int slice = area.getWidth() / 7;
  auto col_area = area.removeFromLeft(slice * 4);
  layout_set_list_songs_and_song_patches(col_area);
  layout_clock_and_notes(area);
}

void MainComponent::layout_set_list_songs_and_song_patches(Rectangle<int> &area)
{
  Grid grid;
  grid.templateRows = { Track(Px(20)), Track(Fr(1)) };
  grid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
  grid.items = {
    // top row labels
    GridItem(set_list_songs_label), GridItem(song_patches_label),
    // top row components
    GridItem(set_list_songs), GridItem(song_patches)
  };
  grid.performLayout(area);
}

void MainComponent::layout_clock_and_notes(Rectangle<int> &area)
{
  auto row_area = area.removeFromTop(MAIN_LABEL_HEIGHT);
  clock_label.setBounds(row_area);
  row_area = area.removeFromTop(CLOCK_INFO_HEIGHT);

  clock_light.setBounds(row_area.removeFromLeft(CLOCK_BUTTON_WIDTH));
  bpm.setBounds(row_area.removeFromLeft(BPM_WIDTH));

  bpm_label.setBounds(row_area.removeFromLeft(BPM_LABEL_WIDTH));

  row_area.removeFromLeft(SPACE);
  clock_button.setBounds(row_area.removeFromLeft(CLOCK_BUTTON_WIDTH));

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(MAIN_LABEL_HEIGHT);
  song_notes_label.setBounds(row_area);
  song_notes.setBounds(area);
}

void MainComponent::layout_middle(Rectangle<int> &area)
{
  Grid grid;
  grid.templateRows = { Track(Px(MAIN_LABEL_HEIGHT)), Track(Fr(1)) };
  grid.templateColumns = { Track(Fr(1)) };
  grid.items = { GridItem(connections_table_label), GridItem(connections_table) };
  grid.performLayout(area);
}

void MainComponent::layout_bottom(Rectangle<int> &area)
{
  Grid bottom_grid;
  bottom_grid.templateRows = { Track(Px(MAIN_LABEL_HEIGHT)), Track(Fr(1)) };
  bottom_grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(2)) };
  bottom_grid.items = {
    // bottom row labels
    GridItem(set_lists_label), GridItem(messages_label), GridItem(triggers_label),
    // bottom row components
    GridItem(set_lists), GridItem(messages), GridItem(triggers)
  };
  bottom_grid.performLayout(area);
}

void MainComponent::update() {
  set_list_songs.updateContent();
  song_patches.updateContent();
  // update_clock_contents();
  song_notes.update_contents();
  connections_table.updateContent();
  set_lists.updateContent();
  messages.updateContent();
  triggers.updateContent();
}

// ================ initial load ================

void MainComponent::load_or_create_keymaster() {
  auto settings = app_properties.getUserSettings();
  if (settings->containsKey(KM_FILE_PROPS_KEY)) {
    File file(settings->getValue(KM_FILE_PROPS_KEY));
    if (!file.exists())
      settings->removeValue(KM_FILE_PROPS_KEY);
    else {
      auto result = loadDocument(file);
      if (result != Result::ok()) {
        AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                                         "Error loading KeyMaster file",
                                         result.getErrorMessage(),
                                         "OK");
      }
    }
  }

  auto km = KeyMaster_instance();
  if (km != nullptr)
    return;

  km = new KeyMaster(device_manager);
  km->initialize();
  km->set_file_based_document(this);
  setChangedFlag(false);   // initialize will set it to true
  km->start();
}

// ================ menu ================

void MainComponent::make_menu_bar() {
  menu_manager.make_menu_bar(this);
}

void MainComponent::create_new_project() {
  KeyMaster *old_km = KeyMaster_instance();
  KeyMaster *new_km = new KeyMaster(device_manager);
  new_km->set_file_based_document(this);
  new_km->initialize();
  setChangedFlag(false);   // initialize has set it to true

  if (old_km != nullptr) {
    old_km->stop();
    delete old_km;
  }
  add_km_listeners();
  new_km->start();

  update();

  // Remove old project, if any, from settings
  auto settings = app_properties.getUserSettings();
  settings->removeValue(KM_FILE_PROPS_KEY);
}

void MainComponent::new_project() {
  saveIfNeededAndUserAgreesAsync([this] (SaveResult result) {
    if (result == SaveResult::savedOk)
      create_new_project();
  });
}

void MainComponent::open_project() {
  // Only load if save is done, so do it in callback
  saveIfNeededAndUserAgreesAsync([this] (FileBasedDocument::SaveResult save_result) {
    if (save_result == SaveResult::savedOk) {
      loadFromUserSpecifiedFileAsync(true, [this] (Result result) {
        // loadDocument handles updating everything
      });
    }
  });
}

void MainComponent::save_project() {
  saveAsync(true, true, [this] (FileBasedDocument::SaveResult save_result) {
    if (save_result == SaveResult::savedOk) {
      auto settings = app_properties.getUserSettings();
      settings->setValue(KM_FILE_PROPS_KEY, getFile().getFullPathName());
    }
  } );
}

void MainComponent::save_project_as() {
  saveAsInteractiveAsync(true, [this] (FileBasedDocument::SaveResult _) { } );
}

void MainComponent::undo() {
  // TODO
}

void MainComponent::redo() {
  // TODO
}

void MainComponent::cut() {
  // TODO
}

void MainComponent::copy() {
  // TODO
}

void MainComponent::paste() {
  // TODO
}

void MainComponent::new_message() {
  Editor e;
  MessageBlock *m = e.create_message();
  // TODO open editor
  e.add_message(m);

  messages.updateContent();
}

void MainComponent::new_trigger() {
  Editor e;
  Trigger *t = e.create_trigger();
  // TODO open editor
  e.add_trigger(t);

  triggers.updateContent();
}

void MainComponent::new_song() {
  open_song_editor(nullptr)->addActionListener(this);
}

void MainComponent::new_patch() {
  Editor e;
  Patch *p = e.create_patch();
  // TODO open editor
  e.add_patch(p);

  song_patches.updateContent();
  connections_table.updateContent();
}

void MainComponent::new_connection() {
  open_connection_editor(nullptr)->addActionListener(&connections_table);
}

void MainComponent::new_set_list() {
  open_set_list_editor(nullptr)->addActionListener(this);
}

void MainComponent::delete_message() {
  int row = messages.getSelectedRow();
  if (row == -1)
    return;

  // TODO
  messages.updateContent();
}

void MainComponent::delete_trigger() {
  // TODO
  triggers.updateContent();
}

void MainComponent::delete_song() {
  // TODO
  set_list_songs.updateContent();
  song_patches.updateContent();
  connections_table.updateContent();
}

void MainComponent::delete_patch() {
  // TODO
  song_patches.updateContent();
  connections_table.updateContent();
}

void MainComponent::delete_connection() {
  // TODO
  connections_table.updateContent();
}

void MainComponent::delete_set_list() {
  // TODO
  set_lists.updateContent();
  set_list_songs.updateContent();
}

void MainComponent::next_song() {
  KeyMaster_instance()->cursor()->next_song();
}

void MainComponent::prev_song() {
  KeyMaster_instance()->cursor()->prev_song();
}

void MainComponent::next_patch() {
  KeyMaster_instance()->cursor()->next_patch();
}

void MainComponent::prev_patch() {
  KeyMaster_instance()->cursor()->prev_patch();
}

void MainComponent::find_song() {
}

void MainComponent::find_set_list() {
}

void MainComponent::toggle_clock() {
  KeyMaster_instance()->toggle_clock();
}

void MainComponent::all_notes_off() {
  KeyMaster_instance()->panic(false);
}

void MainComponent::super_panic() {
  KeyMaster_instance()->panic(true);
}

void MainComponent::midi_monitor() {
  if (_midi_monitor_window != nullptr) {
    _midi_monitor_window->setVisible(true);
    _midi_monitor_window->midi_monitor()->start();
    return;
  }

  // TODO pass rectangle to window constructor
  _midi_monitor_window = std::unique_ptr<MidiMonitorWindow>(new MidiMonitorWindow(app_properties));
  _midi_monitor_window->setVisible(true);
}

// ================ loading and saving ================

void MainComponent::check_ok_to_quit(std::function<void(bool)> callback) {
  if (hasChangedSinceSaved()) {
    saveIfNeededAndUserAgreesAsync([callback] (SaveResult result) {
      callback(result == SaveResult::savedOk);
    });
  }
  else {
    callback(true);
  }
}

Result MainComponent::loadDocument(const File &file) {
  KeyMaster *old_km = KeyMaster_instance();
  Storage s(device_manager, file);

  KeyMaster *new_km = s.load();
  if (s.has_error())            // new_km == old_km, instance already reset
    return Result::fail(s.error());

  new_km->set_file_based_document(this);
  if (old_km != nullptr) {
    old_km->stop();
    delete old_km;
  }
  setFile(file);
  setChangedFlag(false);
  add_km_listeners();
  new_km->start();

  auto settings = app_properties.getUserSettings();
  settings->setValue(KM_FILE_PROPS_KEY, file.getFullPathName());
  update();
  return Result::ok();
}

Result MainComponent::saveDocument(const File &file) {
  Storage s(device_manager, file);
  s.save(KeyMaster_instance());
  if (s.has_error())
    return Result::fail(s.error());
  return Result::ok();
}

// ================ components ================

void MainComponent::config_label(Label &label, const char *text) {
  addAndMakeVisible(label);
  label.setText(text, NotificationType::dontSendNotification);
  label.setFont(FontOptions(16.0f, Font::bold));
  label.setColour(Label::textColourId, Colours::lightgreen);
}

void MainComponent::config_lbox(const char *label_text, Label &label, ListBox &list_box) {
  config_label(label, label_text);
  list_box.setColour(ListBox::outlineColourId, Colours::grey);
  list_box.setOutlineThickness(1);
  addAndMakeVisible(list_box);
}

void MainComponent::config_list_box(
  const char *label_text, Label &label, KmListBox &list_box, KmListBoxModel *model)
{
  box_models.add(model);
  list_box.setModel(model);
  model->addActionListener(&list_box);
  list_box.addActionListener(this);
  config_lbox(label_text, label, list_box);
}

void MainComponent::config_table_list_box(
  const char *label_text, Label &label, KmTableListBox &list_box, KmTableListBoxModel *model)
{
  table_box_models.add(model);
  list_box.setModel(model);
  model->make_columns(list_box.getHeader());
  model->addActionListener(&list_box);
  list_box.addActionListener(this);
  config_lbox(label_text, label, list_box);
}

void MainComponent::update_clock_contents() {
  bpm.setText(String(KeyMaster_instance()->clock().bpm()), NotificationType::dontSendNotification);
}

void MainComponent::make_set_list_songs_pane() {
  auto model = new SetListSongsListBoxModel();
  config_list_box("Set List Songs", set_list_songs_label, set_list_songs, model);
  set_list_songs.selectRow(model->selected_row_num());
}

void MainComponent::make_song_patches_pane() {
  auto model = new SongPatchesListBoxModel();
  config_list_box("Song Patches", song_patches_label, song_patches, model);
  song_patches.selectRow(model->selected_row_num());
}

void MainComponent::make_clock_pane() {
  bpm.setText(String(KeyMaster_instance()->clock().bpm()));
  config_label(clock_label, "Clock");
  addAndMakeVisible(clock_light);
  addAndMakeVisible(bpm);
  addAndMakeVisible(bpm_label);
  addAndMakeVisible(clock_button);
}

void MainComponent::make_song_notes_pane() {
  config_label(song_notes_label, "Song Notes");

  song_notes.update_contents();
  addAndMakeVisible(song_notes);
}

void MainComponent::make_connections_pane() {
  auto model = new ConnectionsTableListBoxModel();
  config_table_list_box("Patch Connections", connections_table_label, connections_table, model);
}

void MainComponent::make_set_lists_pane() {
  auto model = new SetListsListBoxModel();
  config_list_box("Set Lists", set_lists_label, set_lists, model);
  set_lists.selectRow(model->selected_row_num());
}

void MainComponent::make_messages_pane() {
  auto model = new MessagesListBoxModel();
  config_list_box("Messages", messages_label, messages, model);
}

void MainComponent::make_triggers_pane() {
  auto model = new TriggersTableListBoxModel();
  config_table_list_box("Triggers", triggers_label, triggers, model);
}

// ================ helpers ================

// Call this before calling the cursor's init method so we get the "moved"
// message it sends and can redraw everything we need to.
void MainComponent::add_km_listeners() {
  KeyMaster *km = KeyMaster_instance();

  Cursor *cursor = km->cursor();
  cursor->addActionListener(&set_list_songs);
  cursor->addActionListener(&song_patches);
  cursor->addActionListener(&song_notes);
  cursor->addActionListener(&connections_table);
  cursor->addActionListener(&set_lists);

  Clock &clock = km->clock();
  clock.addActionListener(&clock_light);
  clock.addActionListener(&bpm);
  clock.addActionListener(&clock_button);
  
}

void MainComponent::actionListenerCallback(const String &message) {
  if (message == "update:all")
    update();
}
