#pragma once

#include <string>
// #include <sqlite3.h>
#include <JuceHeader.h>
#include "keymaster.h"
#include "midi_device.h"

class Storage {
public:
  Storage(const File &f);
  ~Storage();

  KeyMaster *load(bool testing = false);
  void save(KeyMaster *km);
  bool has_error();
  String error();

private:
  KeyMaster *km;
  const File &file;
  int loading_version;
  String error_str;

  void load_schema_version(var);
  void load_instruments(var);
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
  void assign_ids(Array<DBObj *> objs);
  var schema_version();
  var curves();
  var curves(Array<Curve *> &curves);
  var messages();
  var triggers();
  var songs();
  var patches(Array<Patch *> &patches);
  var connections(Array<Connection *> &connections);
  var controller_mapping(Controller *controller);
  var message_filter(MessageFilter &mf);
  var set_lists();

  void create_default_patches();
  void create_default_patch(Song *);

  MidiInputEntry::Ptr find_input_by_id(const char * const, DBObjID, const String &);
  MidiOutputEntry::Ptr find_output_by_id(const char * const, DBObjID, const String &);
  MessageBlock *find_message_by_id(const char * const, DBObjID, DBObjID);
  Song *find_song_by_id(const char * const, DBObjID, DBObjID);
  void set_find_error_message(const char * const, DBObjID,
                              const char * const, DBObjID);
  void set_find_error_message(const char * const, DBObjID,
                              const char * const, const String &id);

  // // SQL statement helpers
  // int int_or_null(sqlite3_stmt *stmt, int col_num, int null_val=UNDEFINED);
  // DBObjID id_or_null(sqlite3_stmt *stmt, int col_num, DBObjID null_val=UNDEFINED_ID);
  // const char *text_or_null(sqlite3_stmt *stmt, int col_num, const char *null_val);
  // void bind_obj_id_or_null(sqlite3_stmt *stmt, int col_num, DBObj *obj_ptr);
  // void bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval=UNDEFINED);
  // void extract_id(DBObj *db_obj);

  // MidiMessage single_message_from_hex_bytes(char *);
  // String single_message_to_hex_bytes(MidiMessage msg);
};
