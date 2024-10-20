#pragma once

#include <JuceHeader.h>
#include "../km/connection.h"
#include "km_editor.h"

class Connection;
class Controller;

class CcMapEditor : public KmEditor, public Button::Listener {
public:
  CcMapEditor(Connection *conn, Controller *c, bool is_new);
  virtual ~CcMapEditor() {}

  virtual int width() override;
  virtual int height() override;

  virtual void buttonClicked(Button *) override { set_filtered_enabled(); }

private:
  Connection *_conn;
  Controller *_controller;

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

  virtual void layout(Rectangle<int> &area) override;

  virtual void init() override;
  virtual void cancel_cleanup() override;

  void set_filtered_enabled();

  virtual bool apply() override;
};

// If Controller is nullptr we create a new one.
CcMapEditor * open_cc_map_editor(Connection *conn, Controller *c);
