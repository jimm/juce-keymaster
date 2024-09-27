#pragma once

#include <JuceHeader.h>
#include "../km/connection.h"
#include "cc_maps_table.h"
#include "km_table_list_box.h"

class Connection;
class Controller;

class CcMapDialogComponent : public Component {
public:
  CcMapDialogComponent(Connection *conn, Controller *c, bool is_new, KmTableListBox *cc_map_table);

  void resized() override;

  int width();
  int height();

private:
  Connection *_conn;
  Controller *_controller;
  bool _is_new;
  KmTableListBox *_cc_map_table;

  Label _cc_num_label { {}, "Controller Number" };
  TextEditor _cc_num;

  Label _translated_cc_num_label { {}, "Map To:" };
  TextEditor _translated_cc_num;

  Label _filters_label { {}, "Filters" };
  ToggleButton _filtered { "Ignore (input CC ignored, not mapped)" };
  ToggleButton _pass_through_0 { "Pass 0 Unchanged" };
  ToggleButton _pass_through_127 { "Pass 127 Unchanged" };


  Label _min_max_in_label { {}, "Min/Max Input Value" };
  Label _dash1 { {}, " - " };
  TextEditor _min_in;
  TextEditor _max_in;

  Label _min_max_out_label { {}, "Min/Max Output Value" };
  Label _dash2 { {}, " - " };
  TextEditor _min_out;
  TextEditor _max_out;

  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };
  TextButton _apply { "Apply" };

  void init();
  void init_text_editor(TextEditor &te, String initial_contents);

  void layout(Rectangle<int> &area);
  void layout_buttons(Rectangle<int> &area);

  void ok();
  void cancel();
  bool apply();
};

// If Controller is nullptr we create a new one.
void open_cc_map_editor(Connection *conn, Controller *c, KmTableListBox *cc_map_table);
