#include <JuceHeader.h>
#include "keymaster.h"
#include "consts.h"
#include "cursor.h"
#include "storage.h"
#include "formatter.h"
#include "input.h"
#include "output.h"

#define SCHEMA_VERSION 1

Storage::Storage(const File &f) : file(f), loading_version(0) {
}

Storage::~Storage() {
}

// Does not stop or delete the old instance or start the new one.
KeyMaster *Storage::load(bool testing) {
  KeyMaster *curr_km = KeyMaster_instance();

  km = new KeyMaster(testing); // side-effect: KeyMaster static instance set

  var data;
  auto result = JSON::parse(file.loadFileAsString(), data);
  if (result.failed()) {
    error_str <<  "error loading file " << file.getFileName() << ": "
              << result.getErrorMessage();
    delete km;
    set_KeyMaster_instance(curr_km);
    return curr_km;
  }

  auto nullish = var();
  Array<var> arr { nullish };
  auto nullish_arr = var(arr);

  load_schema_version(data.getProperty("schema_version", nullish)); // TODO check version
  load_curves(data.getProperty("curves", nullish));
  load_messages(data.getProperty("messages", nullish));
  load_triggers(data.getProperty("triggers", nullish));
  load_songs(data.getProperty("songs", nullish_arr));
  load_set_lists(data.getProperty("set_lists", nullish_arr));
  create_default_patches();

  return km;
}

// Ignores all in-memory database object ids, generating new ones instead.
// Objects are written in "dependency order" so that all newly assigned ids
// are available for later writing.
void Storage::save(KeyMaster *keymaster) {
  km = keymaster;
  assign_ids();

  DynamicObject obj;
  obj.setProperty("schema_version", schema_version());
  obj.setProperty("messages", messages());
  obj.setProperty("triggers", triggers());
  obj.setProperty("songs", songs());
  obj.setProperty("set_lists", set_lists());
  obj.setProperty("curves", curves());

  FileOutputStream output(file);
  output.setPosition(0);
  output.truncate();
  obj.writeAsJSON(output, JSON::FormatOptions());
}

bool Storage::has_error() {
  return error_str != "";
}

String Storage::error() {
  return error_str;
}

// ================================================================
// load helpers
// ================================================================

void Storage::load_schema_version(var schema_version) {
  // TODO do something with this
}

void Storage::load_curves(var curves) {
  var v;
  for (int i = 0; i < curves.size(); ++i) {
    var vcurve = curves[i];
    Curve *curve = new Curve(
      (int)vcurve.getProperty("id", v),
      (String)vcurve.getProperty("name", "New Curve"),
      (String)vcurve.getProperty("short_name", "nc"));
    var vals = vcurve.getProperty("values", v);
    for (int j = 0; j < 128; ++j)
      curve->curve[j] = (int)vals[j];
    km->curves().add(curve);
  }
}

void Storage::load_messages(var messages) {
  var v;
  for (int i = 0; i < messages.size(); ++i) {
    var vmsg = messages[i];
    MessageBlock *m = new MessageBlock(
      (int)vmsg.getProperty("id", v),
      (String)vmsg.getProperty("name", v));
    auto bytes_str = (String)vmsg.getProperty("bytes", v);
    m->from_hex_string(bytes_str);
    km->messages().add(m);
  }
}

void Storage::load_triggers(var triggers) {
  var v;
  for (int i = 0; i < triggers.size(); ++i) {
    var vmsg = triggers[i];

    TriggerAction action = static_cast<TriggerAction>((int)vmsg.getProperty("action", v));
    Trigger *t = new Trigger(
      (int)vmsg.getProperty("id", v),
      (String)vmsg.getProperty("name", v),
      action,
      nullptr);

    auto bytes_str = (String)vmsg.getProperty("bytes", v);
    if (vmsg.hasProperty("input_identifier")) {
      MessageBlock mblock(UNDEFINED_ID, "");
      mblock.from_hex_string((String)vmsg.getProperty("trigger_message_bytes", v));
      t->set_trigger_message((String)vmsg.getProperty("input_identifier", v),
                             mblock.midi_messages().getEventPointer(0)->message);
    }
    if (vmsg.hasProperty("key_code"))
      t->set_trigger_key_code((int)vmsg.getProperty("key_code", v));
    if (vmsg.hasProperty("output_message_id"))
      t->set_output_message(find_message_by_id("trigger", t->id(), (int)vmsg.getProperty("output_message_id", v)));
    km->triggers().add(t);
  }
}

void Storage::load_songs(var songs) {
  var v;
  for (int i = 0; i < songs.size(); ++i) {
    var vsong = songs[i];
    Song *s = new Song(
      (int)vsong.getProperty("id", v),
      (String)vsong.getProperty("name", v));
    s->set_bpm((float)vsong.getProperty("bpm", v));
    s->set_clock_on_at_start((bool)vsong.getProperty("clock_on_at_start", v));
    s->set_notes((String)vsong.getProperty("notes", v));
    load_patches(s, vsong.getProperty("patches", v));
    km->all_songs()->songs().add(s);
  }
  km->sort_all_songs();
}

void Storage::load_patches(Song *song, var patches) {
  var v;
  for (int i = 0; i < patches.size(); ++i) {
    var vpatch = patches[i];
    Patch *p = new Patch(
      UNDEFINED_ID,
      (String)vpatch.getProperty("name", v));
    if (vpatch.hasProperty("start_message_id"))
      p->set_start_message(find_message_by_id("patch", UNDEFINED_ID,
                                              (int)vpatch.getProperty("start_message_id", v)));
    if (vpatch.hasProperty("stop_message_id"))
      p->set_stop_message(find_message_by_id("patch", UNDEFINED_ID,
                                             (int)vpatch.getProperty("stop_message_id", v)));
    load_connections(p, vpatch.getProperty("connections", v));
    song->patches().add(p);
  }
}

void Storage::create_default_patches() {
  for (auto& song : km->all_songs()->songs())
    if (song->patches().isEmpty())
      create_default_patch(song);
}

// Connects first input to first output. Name of patch == song name.
void Storage::create_default_patch(Song *s) {
  Patch *p = new Patch(UNDEFINED_ID, s->name());
  s->add_patch(p);
  if (km->inputs().isEmpty() || km->outputs().isEmpty())
    return;
  Connection *conn =
    new Connection(UNDEFINED_ID,
                   km->inputs().size() > 0 ? km->inputs()[0] : nullptr,
                   CONNECTION_ALL_CHANNELS,
                   km->outputs().size() > 0 ? km->outputs()[0] : nullptr,
                   CONNECTION_ALL_CHANNELS);
  p->add_connection(conn);
}

void Storage::load_connections(Patch *patch, var connections) {
  var v;
  var undef(UNDEFINED);
  for (int i = 0; i < connections.size(); ++i) {
    var vconn = connections[i];
    String input_identifier = (String)vconn.getProperty("input_id", v);
    String output_identifier = (String)vconn.getProperty("output_id", v);
    Connection *c = new Connection(UNDEFINED_ID,
                                   find_input_by_id("conn", UNDEFINED_ID, input_identifier),
                                   (int)vconn.getProperty("input_chan", undef),
                                   find_output_by_id("conn", UNDEFINED_ID, output_identifier),
                                   (int)vconn.getProperty("output_chan", undef));

    c->set_program_bank_msb((int)vconn.getProperty("bank_msb", undef));
    c->set_program_bank_lsb((int)vconn.getProperty("bank_lsb", undef));
    c->set_program_prog((int)vconn.getProperty("program", undef));
    var zone_arr = vconn.getProperty("zone", v);
    c->set_zone_low((int)zone_arr[0]);
    c->set_zone_high((int)zone_arr[1]);
    c->set_xpose((int)vconn.getProperty("xpose", v));

    int vcurve_id = (int)vconn.getProperty("velocity_curve_id", v);
    for (auto &curve : km->curves())
      if (curve->id() == vcurve_id) {
        c->set_velocity_curve(curve);
        break;
      }

    load_message_filter(c, vconn.getProperty("message_filter", v));
    load_controller_mappings(c, vconn);

    patch->connections().add(c);
  }
}

void Storage::load_message_filter(Connection *conn, var vmf) {
  auto mf = conn->message_filter();
  int flags = (int)vmf;
  mf.set_note((flags & (1 << 0)) != 0);
  mf.set_poly_pressure((flags & (1 << 1)) != 0);
  mf.set_chan_pressure((flags & (1 << 2)) != 0);
  mf.set_program_change((flags & (1 << 3)) != 0);
  mf.set_pitch_bend((flags & (1 << 4)) != 0);
  mf.set_controller((flags & (1 << 5)) != 0);
  mf.set_song_pointer((flags & (1 << 6)) != 0);
  mf.set_song_select((flags & (1 << 7)) != 0);
  mf.set_tune_request((flags & (1 << 8)) != 0);
  mf.set_sysex((flags & (1 << 9)) != 0);
  mf.set_clock((flags & (1 << 10)) != 0);
  mf.set_start_continue_stop((flags & (1 << 11)) != 0);
  mf.set_system_reset((flags & (1 << 12)) != 0);
}

void Storage::load_controller_mappings(Connection *conn, var vconn) {
  if (!vconn.hasProperty("controller_mappings"))
    return;

  var v;
  var arr = vconn.getProperty("controller_mappings", v);
  for (int i = 0; i < arr.size(); ++i) {
    var cmap = arr[i];
    int cc_num = (int)cmap.getProperty("cc_num", v);
    Controller *c = new Controller(UNDEFINED, cc_num);
    c->set_cc_num((int)cmap.getProperty("cc_num", v));
    c->set_translated_cc_num((int)cmap.getProperty("translated_cc_num", v));
    c->set_filtered((bool)cmap.getProperty("filtered", v));
    c->set_range(
      (bool)cmap.getProperty("pass_through_0", v),
      (bool)cmap.getProperty("pass_through_127", v),
      (int)cmap.getProperty("min_in", v),
      (int)cmap.getProperty("max_in", v),
      (int)cmap.getProperty("min_out", v),
      (int)cmap.getProperty("max_out", v));
    conn->set_cc_map(cc_num, c);
  }
}

void Storage::load_set_lists(var slists) {
  var v;

  for (int i = 0; i < slists.size(); ++i) {
    var vsl = slists[i];
    SetList *sl = new SetList(UNDEFINED, (String)vsl.getProperty("name", v));
    var song_id_arr = vsl.getProperty("song_ids", v);
    for (int j = 0; j < song_id_arr.size(); ++j) {
      int song_id = (int)song_id_arr[j];
      sl->songs().add(find_song_by_id("set_list", UNDEFINED, song_id));
    }
    km->set_lists().add(sl);
  }
}

// ================================================================
// save helpers
// ================================================================

void Storage::assign_ids(Array<DBObj *> objs) {
  int i = 1;
  for (auto &obj : objs)
    obj->set_id(i++);
}

void Storage::assign_ids() {
  int i = 1;
  for (auto &curve : km->curves())
    curve->set_id(i++);
  i = 0;
  for (auto &message : km->messages())
    message->set_id(i++);
  i = 0;
  for (auto &trigger : km->triggers())
    trigger->set_id(i++);
  i = 0;
  for (auto &song : km->all_songs()->songs())
    song->set_id(i++);
}

var Storage::schema_version() {
  return var(SCHEMA_VERSION);
}

var Storage::curves() {
  return curves(km->curves());
}

var Storage::curves(Array<Curve *> &curves) {
  Array<var> curve_vars;
  for (auto curve : curves) {
    DynamicObject::Ptr c(new DynamicObject());
    c->setProperty("id", var(curve->id()));
    c->setProperty("name", var(curve->name()));
    c->setProperty("short_name", var(curve->short_name()));
    Array<var> values;
    for (int i = 0; i < 128; ++i)
      values.add(var(curve->curve[i]));
    c->setProperty("values", values);
    curve_vars.add(var(c.get()));
  }
  return var(curve_vars);
}

var Storage::messages() {
  Array<var> messages;
  for (auto message : km->messages()) {
    DynamicObject::Ptr m(new DynamicObject());
    m->setProperty("id", var(message->id()));
    m->setProperty("name", var(message->name()));
    m->setProperty("bytes", var(message->to_hex_string()));
    messages.add(var(m.get()));
  }
  return var(messages);
}

var Storage::triggers() {
  Array<var> triggers;
  for (auto trigger : km->triggers()) {
    DynamicObject::Ptr t(new DynamicObject());
    t->setProperty("id", var(trigger->id()));
    t->setProperty("name", var(trigger->name()));

    int action_int = (int)trigger->action();
    t->setProperty("action", var(action_int));

    if (trigger->trigger_input_identifier().isNotEmpty()) {
      t->setProperty("input_identifier", var(trigger->trigger_input_identifier()));
      t->setProperty("trigger_message_bytes",
                     var(String::toHexString(trigger->trigger_message().getRawData(),
                                             trigger->trigger_message().getRawDataSize())));
    }
    if (trigger->trigger_key_code() != UNDEFINED)
      t->setProperty("key_code", var(trigger->trigger_key_code()));
    if (trigger->output_message())
      t->setProperty("output_message_id", var(trigger->output_message()->id()));
    triggers.add(var(t.get()));
  }
  return var(triggers);
}

var Storage::songs() {
  Array<var> songs;
  for (auto song : km->all_songs()->songs()) {
    DynamicObject::Ptr s(new DynamicObject());
    s->setProperty("id", var(song->id()));
    s->setProperty("name", var(song->name()));
    s->setProperty("bpm", var(song->bpm()));
    s->setProperty("clock_on_at_start", var(song->clock_on_at_start()));
    s->setProperty("notes", var(song->notes()));
    s->setProperty("patches", patches(song->patches()));
    songs.add(var(s.get()));
  }
  return var(songs);
}

var Storage::patches(Array<Patch *> &patches) {
  Array<var> ps;
  for (auto patch : patches) {
    DynamicObject::Ptr p(new DynamicObject());
    p->setProperty("name", var(patch->name()));
    p->setProperty("connections", var(connections(patch->connections())));
    if (patch->start_message())
      p->setProperty("start_message_id", patch->start_message()->id());
    if (patch->stop_message())
      p->setProperty("stop_message_id", patch->stop_message()->id());
    ps.add(var(p.get()));
  }
  return var(ps);
}

var Storage::connections(Array<Connection *> &connections) {
  Array<var> cs;
  for (auto conn : connections) {
    DynamicObject::Ptr c(new DynamicObject());
    c->setProperty("input_id", conn->input()->identifier());
    c->setProperty("output_id", conn->output()->identifier());
    if (conn->input_chan() != UNDEFINED)
      c->setProperty("input_chan", conn->input_chan());
    if (conn->output_chan() != UNDEFINED)
      c->setProperty("output_chan", conn->output_chan());

    DynamicObject::Ptr prog(new DynamicObject());
    if (conn->program_bank_msb() != UNDEFINED)
      prog->setProperty("bank_msb", var(conn->program_bank_msb()));
    if (conn->program_bank_lsb() != UNDEFINED)
      prog->setProperty("bank_msb", var(conn->program_bank_lsb()));
    if (conn->program_bank_msb() != UNDEFINED)
      prog->setProperty("program", var(conn->program_prog()));
    c->setProperty("program", var(prog.get()));

    DynamicObject::Ptr zone(new DynamicObject());
    Array<var> zone_arr = { var(conn->zone_low()), var(conn->zone_high()) };
    c->setProperty("zone", var(zone_arr));

    c->setProperty("xpose", conn->xpose());
    if (conn->velocity_curve() != nullptr)
      c->setProperty("velocity_curve_id", conn->velocity_curve()->id());
    c->setProperty("message_filter", message_filter(conn->message_filter()));

    Array<var> cc_maps;
    for (int i = 0; i < 128; ++i) {
      Controller *cc_map = conn->cc_map(i);
      if (cc_map != nullptr)
        cc_maps.add(controller_mapping(cc_map));
    }
    if (!cc_maps.isEmpty())
      c->setProperty("controller_mappings", var(cc_maps));
    
    cs.add(var(c.get()));
  }
  return var(cs);
}

var Storage::controller_mapping(Controller *controller) {
  DynamicObject::Ptr c(new DynamicObject());

  c->setProperty("cc_num", var(controller->cc_num()));
  c->setProperty("translated_cc_num", var(controller->translated_cc_num()));
  c->setProperty("filtered", var(controller->filtered()));
  c->setProperty("pass_through_0", var(controller->pass_through_0()));
  c->setProperty("pass_through_127", var(controller->pass_through_127()));
  c->setProperty("min_in", var(controller->min_in()));
  c->setProperty("max_in", var(controller->max_in()));
  c->setProperty("min_out", var(controller->min_out()));
  c->setProperty("max_out", var(controller->max_out()));
  
  return var(c.get());
}

var Storage::message_filter(MessageFilter &mf) {
  int flags = 0;
  if (mf.note()) flags |= (1 << 0);
  if (mf.poly_pressure()) flags |= (1 << 1);
  if (mf.chan_pressure()) flags |= (1 << 2);
  if (mf.program_change()) flags |= (1 << 3);
  if (mf.pitch_bend()) flags |= (1 << 4);
  if (mf.controller()) flags |= (1 << 5);
  if (mf.song_pointer()) flags |= (1 << 6);
  if (mf.song_select()) flags |= (1 << 7);
  if (mf.tune_request()) flags |= (1 << 8);
  if (mf.sysex()) flags |= (1 << 9);
  if (mf.clock()) flags |= (1 << 10);
  if (mf.start_continue_stop()) flags |= (1 << 11);
  if (mf.system_reset()) flags |= (1 << 12);
  return var(flags);
}

var Storage::set_lists() {
  Array<var> set_lists;

  for (auto slist : km->set_lists()) {
    if (slist == km->all_songs())
      continue;

    DynamicObject::Ptr sl(new DynamicObject());
    sl->setProperty("name", slist->name());
    Array<var> song_ids;
    for (auto song : slist->songs())
      song_ids.add(var((int)song->id()));
    sl->setProperty("song_ids", var(song_ids));
    set_lists.add(var(sl.get()));
  }

  return var(set_lists);
}

// ================================================================
// find by id
// ================================================================

Input::Ptr Storage::find_input_by_id(
  const char * const searcher_name, DBObjID searcher_id, const String &id
) {
  Input::Ptr input = Input::find_by_id(id);
  if (input)
    return input;

  set_find_error_message(searcher_name, searcher_id, "input", id);
  return nullptr;
}

Output::Ptr Storage::find_output_by_id(
  const char * const searcher_name, DBObjID searcher_id, const String &id
) {
  Output::Ptr output = Output::find_by_id(id);
  if (output)
    return output;

  set_find_error_message(searcher_name, searcher_id, "output", id);
  return nullptr;
}

MessageBlock *Storage::find_message_by_id(
  const char * const searcher_name, DBObjID searcher_id, DBObjID id
) {
  for (auto msg : km->messages())
    if (msg->id() == id)
      return msg;
  set_find_error_message(searcher_name, searcher_id, "message", id);
  return nullptr;
}

Song *Storage::find_song_by_id(
  const char * const searcher_name, DBObjID searcher_id, DBObjID id
) {
  for (auto &song : km->all_songs()->songs())
    if (song->id() == id)
        return song;
  set_find_error_message(searcher_name, searcher_id, "song", id);
  return nullptr;
}

void Storage::set_find_error_message(
  const char * const searcher_name, DBObjID searcher_id,
  const char * const find_name, DBObjID find_id
) {
  error_str = String::formatted("%s (%lld) can't find %s with id %lld",
          searcher_name, searcher_id, find_name, find_id);
}

void Storage::set_find_error_message(
  const char * const searcher_name, DBObjID searcher_id,
  const char * const find_name, const String &find_id
) {
  error_str = String::formatted("%s (%lld) can't find %s with id ");
  error_str += find_id;
}

// ================================================================

// int Storage::int_or_null(sqlite3_stmt *stmt, int col_num, int null_val) {
//   return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
//     ? null_val
//     : sqlite3_column_int(stmt, col_num);
// }

// sqlite3_int64 Storage::id_or_null(sqlite3_stmt *stmt, int col_num, sqlite3_int64 null_val) {
//   return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
//     ? null_val
//     : sqlite3_column_int64(stmt, col_num);
// }

// const char *Storage::text_or_null(sqlite3_stmt *stmt, int col_num, const char *null_val) {
//   return sqlite3_column_type(stmt, col_num) == SQLITE_NULL
//     ? null_val
//     : (const char *)sqlite3_column_text(stmt, col_num);
// }

// void Storage::bind_obj_id_or_null(sqlite3_stmt *stmt, int col_num, DBObj *obj_ptr) {
//   if (obj_ptr == nullptr || obj_ptr->id() == UNDEFINED_ID)
//     sqlite3_bind_null(stmt, col_num);
//   else
//     sqlite3_bind_int64(stmt, col_num, obj_ptr->id());
// }

// void Storage::bind_int_or_null(sqlite3_stmt *stmt, int col_num, int val, int nullval) {
//   if (val == nullval)
//     sqlite3_bind_null(stmt, col_num);
//   else
//     sqlite3_bind_int(stmt, col_num, val);
// }

// void Storage::extract_id(DBObj *db_obj) {
//   db_obj->set_id(sqlite3_last_insert_rowid(db));
// }

// // FIXME use standard Message output format 007f35b0
// //
// // Parses a single array of chars of length <= 6 representing a single
// // non-separated MIDI messages like 'b0357f'. Returns a MidiMessage. If
// // `bytes` is null, returns a message containing all zeroes.
// MidiMessage Storage::single_message_from_hex_bytes(char *bytes) {
//   if (bytes == nullptr)
//     return Pm_Message(0, 0, 0);

//   // FIXME same as inner loop of messages_from_chars in message.cpp
//   unsigned char byte;
//   MidiMessage msg = 0;

//   for (int i = 0; i < 4; ++i) {
//     msg <<= 8;
//     msg += hex_to_byte(bytes);
//     bytes += 2;
//   }

//   return msg;
// }

// // FIXME use standard Message format 007f35b0
// String Storage::single_message_to_hex_bytes(MidiMessage msg) {
//   char buf[16], *p = buf;

//   for (int i = 0; i < 4; ++i) {
//     snprintf(p, 3, "%02x", (unsigned char)(msg & 0xff));
//     msg >>= 8;
//     p += 2;
//   }
//   return string(buf);
// }
