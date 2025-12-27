#pragma once

#include <string>
#include <JuceHeader.h>
#include "device_manager.h"
#include "keymaster.h"
#include "input.h"
#include "output.h"

class Storage {
public:
  Storage(DeviceManager &device_manager, const File &f);
  ~Storage();

  KeyMaster *load(bool testing = false);
  void save(KeyMaster *km);
  bool has_error();
  String error();

private:
  DeviceManager &device_manager;
  KeyMaster *km;
  const File &file;
  int loading_version;
  String error_str;

  void load_schema_version(var);
  void load_curves(var);
  void load_messages(var);
  void load_triggers(var);
  void load_songs(var);
  void load_patches(Song *, var);
  void load_connections(Patch *, var);
  void load_message_filter(Connection *, var);
  void load_controller_mappings(Connection *, var);
  void load_set_lists(var);

  void assign_ids();
  var schema_version_var();
  var curves_var();
  var messages_var();
  var triggers_var();
  var songs_var();
  var patches_var(Array<Patch *> &patches);
  var connections_var(Array<Connection *> &connections);
  var controller_mapping_var(Controller *controller);
  var message_filter_var(MessageFilter &mf);
  var set_lists_var();

  void create_default_patches();
  void create_default_patch(Song *);

  Input::Ptr find_input(const String &, const String &);
  Output::Ptr find_output(const String &, const String &);
  MessageBlock *find_message_by_id(const char * const, DBObjID, DBObjID);
  Song *find_song_by_id(const char * const, DBObjID, DBObjID);
  void set_find_error_message(const char * const, DBObjID,
                              const char * const, DBObjID);
  void set_find_error_message(const char * const, DBObjID,
                              const char * const, const String &id);
};
