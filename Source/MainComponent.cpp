#include <JuceHeader.h>
#include "MainComponent.h"
#include "km/editor.h"
#include "km/keymaster.h"
#include "km/storage.h"
#include "gui/connection_dialog_component.h"

#define DEFAULT_WINDOW_WIDTH 900
#define DEFAULT_WINDOW_HEIGHT 700

MainComponent::MainComponent(DeviceManager &dev_mgr, ApplicationProperties &props)
  : FileBasedDocument(".kmst", "*.kmst", "Open project", "Save project"),
    device_manager(dev_mgr)
{
  KeyMaster *km = new KeyMaster(device_manager); // side-effect: KeyMaster static instance set
  km->set_file_based_document(this);
  km->initialize();        // generate default curves and initial song/patch
  setChangedFlag(false);   // initialize will set it to true
  km->start();

  make_menu_bar();
  make_set_list_songs_pane();
  make_song_patches_pane();
  make_song_notes_pane();
  make_set_lists_pane();
  make_messages_pane();
  make_triggers_pane();
  make_connections_pane();

  auto settings = props.getUserSettings();
  auto width = settings->getIntValue("window.width", DEFAULT_WINDOW_WIDTH);
  auto height = settings->getIntValue("window.height", DEFAULT_WINDOW_HEIGHT);
  setSize(width, height);
}

MainComponent::~MainComponent()
{
  if (KeyMaster_instance())
    delete KeyMaster_instance();
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

  using Track = Grid::TrackInfo;
  using Fr = Grid::Fr;
  using Px = Grid::Px;

  Grid top_grid;
  top_grid.templateRows = { Track(Px(20)), Track(Fr(1)) };
  top_grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  top_grid.items = {
    // top row labels
    GridItem(set_list_songs_label), GridItem(song_patches_label), GridItem(song_notes_label),
    // top row components
    GridItem(set_list_songs), GridItem(song_patches), GridItem(song_notes),
  };
  // USE THIS BEFORE PERFORM LAYOUT?
  // top_grid.setGap(Px(4));
  top_grid.performLayout(top_area);

  Grid middle_grid;
  middle_grid.templateRows = { Track(Px(20)), Track(Fr(1)) };
  middle_grid.templateColumns = { Track(Fr(1)) };
  middle_grid.items = { GridItem(connections_table_label), GridItem(connections_table) };
  middle_grid.performLayout(middle_area);

  Grid bottom_grid;
  bottom_grid.templateRows = { Track(Px(20)), Track(Fr(1)) };
  bottom_grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  bottom_grid.items = {
    // bottom row labels
    GridItem(set_lists_label), GridItem(messages_label), GridItem(triggers_label),
    // bottom row components
    GridItem(set_lists), GridItem(messages), GridItem(triggers)
  };
  bottom_grid.performLayout(bottom_area);
}

void MainComponent::update() {
  set_list_songs.updateContent();
  song_patches.updateContent();
  connections_table.updateContent();
  set_lists.updateContent();
  messages.updateContent();
  triggers.updateContent();
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
  new_km->start();

  update();
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
        if ((bool)result)
          update();
      });
    }
  });
}

void MainComponent::save_project() {
  saveAsync(true, true, [this] (FileBasedDocument::SaveResult _) { } );
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
  Editor e;
  Song *s = e.create_song();
  // TODO open editor
  e.add_song(s);

  set_list_songs.updateContent();
  song_patches.updateContent();
  connections_table.updateContent();
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
  Patch *p = KeyMaster_instance()->cursor()->patch();
  open_connection_editor(p, nullptr)->addActionListener(&connections_table);
}

void MainComponent::new_set_list() {
  Editor e;
  SetList *set_list = e.create_set_list();
  e.add_set_list(set_list);
  // TODO open editor

  set_lists.updateContent();
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
}

// ================ loading and saving ================

void MainComponent::check_ok_to_quit(std::function<void(bool)> callback) {
  if (!hasChangedSinceSaved())
    callback(true);
  saveIfNeededAndUserAgreesAsync([callback] (SaveResult result) {
    callback(result != SaveResult::failedToWriteToFile);
  });
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
  new_km->start();

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
  model->set_list_box(&list_box);
  config_lbox(label_text, label, list_box);
}

void MainComponent::config_table_list_box(
  const char *label_text, Label &label, KmTableListBox &list_box, KmTableListBoxModel *model)
{
  table_box_models.add(model);
  list_box.setModel(model);
  model->addActionListener(&list_box);
  model->set_list_box(&list_box);
  model->make_columns(list_box.getHeader());
  config_lbox(label_text, label, list_box);
}

void MainComponent::make_set_list_songs_pane() {
  auto model = new SetListSongsListBoxModel();
  config_list_box("Set List Songs", set_list_songs_label, set_list_songs, model);
  KeyMaster_instance()->cursor()->addActionListener(&set_list_songs);
  set_list_songs.selectRow(model->selected_row_num());
}

void MainComponent::make_song_patches_pane() {
  auto model = new SongPatchesListBoxModel();
  config_list_box("Song Patches", song_patches_label, song_patches, model);
  KeyMaster_instance()->cursor()->addActionListener(&song_patches);
  song_patches.selectRow(model->selected_row_num());
}

void MainComponent::make_song_notes_pane() {
  config_label(song_notes_label, "Song Notes");
  song_notes.setJustificationType(Justification(Justification::topLeft));

  song_notes.update_contents();
  KeyMaster_instance()->cursor()->addActionListener(&song_notes);
  addAndMakeVisible(song_notes);
}

void MainComponent::make_connections_pane() {
  auto model = new ConnectionsTableListBoxModel();
  config_table_list_box("Patch Connections", connections_table_label, connections_table, model);
  KeyMaster_instance()->cursor()->addActionListener(&connections_table);
}

void MainComponent::make_set_lists_pane() {
  auto model = new SetListsListBoxModel();
  config_list_box("Set Lists", set_lists_label, set_lists, model);
  KeyMaster_instance()->cursor()->addActionListener(&set_lists);
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
