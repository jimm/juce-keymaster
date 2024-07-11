#include "MainComponent.h"
#include "km/keymaster.h"

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

MainComponent::MainComponent(juce::ApplicationProperties &props)
{
  KeyMaster *km = new KeyMaster(); // side-effect: KeyMaster static instance set
  km->initialize();        // generate default curves and initial song/patch
  km->start();

  make_menu_bar();
  make_set_list_songs_pane();
  make_song_patches_pane();
  make_song_notes_pane();
  make_set_lists_pane();
  make_messages_pane();
  make_triggers_pane();
  make_patch_pane();

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
  auto top_area = area.removeFromTop((area.getHeight() * 2 / 3) - 2);
  auto bottom_area = area.removeFromBottom(area.getHeight() - 2);

  Grid top_grid;
  using Track = Grid::TrackInfo;
  using Fr = Grid::Fr;
  using Px = Grid::Px;

  top_grid.templateRows = {
    Track(Px(20)), Track(Fr(2)),
    Track(Px(20)), Track(Fr(1)),
  };
  top_grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  top_grid.setGap(Px(4));

  top_grid.items = {
    // top row labels
    GridItem(set_list_songs_label), GridItem(song_patches_label), GridItem(song_notes_label),
    // top row components
    GridItem(set_list_songs), GridItem(song_patches), GridItem(song_notes),
    // middle row labels
    GridItem(set_lists_label), GridItem(messages_label), GridItem(triggers_label),
    // middle row components
    GridItem(set_lists), GridItem(messages), GridItem(triggers)
  };

  top_grid.performLayout(top_area);

  Grid bottom_grid;
  bottom_grid.templateRows = { Track(Px(20)), Track(Fr(1)) };
  bottom_grid.templateColumns = { Track(Fr(1)) };

  bottom_grid.items = { GridItem(patch_table_label), GridItem(patch_table) };
  bottom_grid.performLayout(bottom_area);
}

// ================ menu ================

void MainComponent::make_menu_bar() {
  menu_manager.make_menu_bar(this);
}

void MainComponent::new_project() {
}

void MainComponent::open_project() {
}

void MainComponent::close_project() {
}

void MainComponent::save_project() {
}

void MainComponent::save_project_as() {
}

void MainComponent::undo() {
}

void MainComponent::redo() {
}

void MainComponent::cut() {
}

void MainComponent::copy() {
}

void MainComponent::paste() {
}

void MainComponent::new_message() {
}

void MainComponent::new_trigger() {
}

void MainComponent::new_song() {
}

void MainComponent::new_patch() {
}

void MainComponent::new_connection() {
}

void MainComponent::new_set_list() {
}

void MainComponent::delete_message() {
}

void MainComponent::delete_trigger() {
}

void MainComponent::delete_song() {
}

void MainComponent::delete_patch() {
}

void MainComponent::delete_connection() {
}

void MainComponent::delete_set_list() {
}

void MainComponent::next_song() {
}

void MainComponent::prev_song() {
}

void MainComponent::next_patch() {
}

void MainComponent::prev_patch() {
}

void MainComponent::find_song() {
}

void MainComponent::find_set_list() {
}

void MainComponent::toggle_clock() {
}

void MainComponent::all_notes_off() {
}

void MainComponent::super_panic() {
}

void MainComponent::midi_monitor() {
}

// ================ components ================

void MainComponent::config_label(Label &label, const char *text) {
  addAndMakeVisible(label);
  label.setText(text, NotificationType::dontSendNotification);
  label.setFont(FontOptions(16.0f, Font::bold));
  label.setColour(Label::textColourId, Colours::lightgreen);
}

void MainComponent::config_list_box(const char *label_text, Label &label, KmListBox &list_box, KmListBoxModel *model) {
  config_label(label, label_text);
  box_models.add(model);
  list_box.setModel(model);
  addAndMakeVisible(list_box);
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

  Song *song = KeyMaster_instance()->cursor()->song();
  // FIXME need to update this when the song changes
  if (song)
    song_notes.setText(song->notes(), NotificationType::dontSendNotification);
  addAndMakeVisible(song_notes);
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
  auto model = new TriggersListBoxModel();
  config_list_box("Triggers", triggers_label, triggers, model);
}

void MainComponent::make_patch_pane() {
  patch_table = new PatchTableListBoxModel();
  table_box_models.add(patch_table);
  config_label(patch_table_label, "Patch Connections");
  addAndMakeVisible(patch_table);
}
