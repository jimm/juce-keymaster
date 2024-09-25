#pragma once

#include <JuceHeader.h>
#include "../km/connection.h"
#include "connections_table.h"
#include "km_table_list_box.h"

class Patch;

class ConnectionDialogComponent : public Component {
public:
  ConnectionDialogComponent(Patch *p, Connection *c, bool is_new, KmTableListBox *connections_table);

  void resized() override;

  int width();
  int height();

private:
  Patch *_patch;
  Connection *_conn;
  bool _is_new;
  KmTableListBox *_connections_table;

  Label _input_inst_label { {}, "Input" };
  ComboBox _input_instrument;

  Label _input_chan_label { {}, "Input Chan" };
  ComboBox _input_chan;

  Label _output_inst_label { {}, "Output" };
  ComboBox _output_instrument;

  Label _output_chan_label { {}, "Output Chan" };
  ComboBox _output_chan;

  Label _prog_label { {}, "Program Change (in or out channel must be selected)" };
  Label _msb_label { {}, "MSB:" };
  TextEditor _msb { "Bank MSB" };
  Label _lsb_label { {}, "LSB:" };
  TextEditor _lsb { "Bank LSB" };
  Label _prog_field_label { {}, "Prog:" };
  TextEditor _prog { "Program" };

  Label _zone_label { {}, "Keyboard Zone" };
  TextEditor _zone_low { "Zone Low" };
  Label _zone_between { {}, "-" };
  TextEditor _zone_high { "Zone High" };

  Label _xpose_label { {}, "Transpose" };
  TextEditor _xpose { "Tranpose" };

  Label _vc_label { {}, "Velocity Curve" };
  ComboBox _vc;

  // TODO curve, filter, cc map

  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };
  TextButton _apply { "Apply" };

  void layout_instrument(
    Rectangle<int> &area, Label &inst_label, ComboBox &inst, Label &chan_label, ComboBox &chan
  );
  void layout_program(Rectangle<int> &area);
  void layout_zone(Rectangle<int> &area);
  void layout_xpose_and_velocity_curve(Rectangle<int> &area);
  void layout_buttons(Rectangle<int> &area);

  void init_input();
  void init_output();
  void init_prog();
  void init_zone();
  void init_xpose();
  void init_velocity_curve();

  void init_text_editor(TextEditor &te, String initial_contents);

  void ok();
  void cancel();
  bool apply();
};
