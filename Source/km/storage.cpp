#include <JuceHeader.h>
#include "keymaster.h"
#include "consts.h"
#include "device_manager.h"
#include "storage.h"
#include "formatter.h"
#include "input.h"
#include "output.h"
#include "utils.h"

#define SCHEMA_VERSION 1

Storage::Storage(DeviceManager &dev_mgr, const File &f)
  : device_manager(dev_mgr), file(f), loading_version(0)
{
}

Storage::~Storage() {
}

// Does not stop or delete the old instance or start the new one.
KeyMaster *Storage::load(bool testing) {
  KeyMaster *curr_km = KeyMaster_instance();

  km = new KeyMaster(device_manager, testing); // side-effect: KeyMaster static instance set

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
  Array<var> arr;
  auto nullish_arr = var(arr);

  load_schema_version(data.getProperty("schema_version", nullish)); // TODO check version
  load_curves(data.getProperty("curves", nullish));
  load_messages(data.getProperty("messages", nullish));
  load_triggers(data.getProperty("triggers", nullish));
  load_songs(data.getProperty("songs", nullish_arr));
  load_set_lists(data.getProperty("set_lists", nullish_arr));
  create_default_patches();

  km->reset_changed();
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
      curve->curve[j] = (unsigned char)(int)vals[j];
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
    auto result = m->from_hex_string(bytes_str);
    if (result.failed()) {
      // TODO this should be a warning message
      // error_str = "error in message data for message" + m->name();
    }
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
    if (vmsg.hasProperty("trigger_message_bytes")) {
      MessageBlock mblock(UNDEFINED_ID, "");
      auto result = mblock.from_hex_string((String)vmsg.getProperty("trigger_message_bytes", v));
      if (result.failed()) {
        // TODO this should be a warning message, not an error message
        // error_str = String::formatted("error in trigger message data for trigger %d", i + 1);
      }
      else
        t->set_trigger_message(mblock.midi_messages()[0]);
    }
    if (vmsg.hasProperty("key_code"))
      t->set_trigger_key_press(KeyPress((int)vmsg.getProperty("key_code", v)));
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
    km->all_songs()->add_song(s);
  }
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
  if (device_manager.inputs().isEmpty() || device_manager.outputs().isEmpty())
    return;
  Connection *conn =
    new Connection(UNDEFINED_ID,
                   device_manager.inputs().size() > 0 ? device_manager.inputs()[0] : nullptr,
                   CONNECTION_ALL_CHANNELS,
                   device_manager.outputs().size() > 0 ? device_manager.outputs()[0] : nullptr,
                   CONNECTION_ALL_CHANNELS);
  p->add_connection(conn);
}

void Storage::load_connections(Patch *patch, var connections) {
  var v;
  var undef(UNDEFINED);
  var all_chans(CONNECTION_ALL_CHANNELS);
  for (int i = 0; i < connections.size(); ++i) {
    var vconn = connections[i];
    String input_identifier = (String)vconn.getProperty("input_id", v);
    String input_name = (String)vconn.getProperty("input_name", v);
    String output_identifier = (String)vconn.getProperty("output_id", v);
    String output_name = (String)vconn.getProperty("output_name", v);
    Connection *c = new Connection(UNDEFINED_ID,
                                   find_input(input_identifier, input_name),
                                   (int)vconn.getProperty("input_chan", all_chans),
                                   find_output(output_identifier, output_name),
                                   (int)vconn.getProperty("output_chan", all_chans));

    var prog = vconn.getProperty("program", v);
    c->set_program_bank_msb((int)prog.getProperty("bank_msb", undef));
    c->set_program_bank_lsb((int)prog.getProperty("bank_lsb", undef));
    c->set_program_prog((int)prog.getProperty("program", undef));

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
    c->set_cc_num(cc_num);
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

    if (trigger->has_trigger_message()) {
      t->setProperty("trigger_message_bytes",
                     var(String::toHexString(trigger->trigger_message().getRawData(),
                                             trigger->trigger_message().getRawDataSize())));
    }
    if (trigger->has_trigger_key_press())
      t->setProperty("key_code", var(trigger->trigger_key_press().getKeyCode()));
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
    c->setProperty("input_name", conn->input()->name());
    c->setProperty("output_id", conn->output()->identifier());
    c->setProperty("output_name", conn->output()->name());
    if (conn->input_chan() != CONNECTION_ALL_CHANNELS)
      c->setProperty("input_chan", conn->input_chan());
    if (conn->output_chan() != CONNECTION_ALL_CHANNELS)
      c->setProperty("output_chan", conn->output_chan());

    DynamicObject::Ptr prog(new DynamicObject());
    if (conn->program_bank_msb() != UNDEFINED)
      prog->setProperty("bank_msb", var(conn->program_bank_msb()));
    if (conn->program_bank_lsb() != UNDEFINED)
      prog->setProperty("bank_lsb", var(conn->program_bank_lsb()));
    if (conn->program_prog() != UNDEFINED)
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

Input::Ptr Storage::find_input(const String &id, const String &name) {
  return device_manager.find_or_create_input(id, name);
}

Output::Ptr Storage::find_output(const String &id, const String &name) {
  return device_manager.find_or_create_output(id, name);
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
