#pragma once

#include <JuceHeader.h>
#include "../km/connection.h"
#include "km_editor.h"

class ProgramChangeEditor : public KmEditor {
public:
  ProgramChangeEditor(Connection *conn, ProgramChange *pc, bool is_new);
  virtual ~ProgramChangeEditor() {}

  virtual int width() override;
  virtual int height() override;

private:
  Connection *_conn;
  ProgramChange *_pc;

  Label _output_label { {}, "Output Instrument" };
  ComboBox _output_instrument;

  Label _chan_label { {}, "Channel" };
  ComboBox _chan;

  Label _msb_label { {}, "Bank MSB (0-127, blank = none)" };
  TextEditor _msb { "Bank MSB" };

  Label _lsb_label { {}, "Bank LSB (0-127, blank = none)" };
  TextEditor _lsb { "Bank LSB" };

  Label _prog_label { {}, "Program (0-127, blank = none)" };
  TextEditor _prog { "Program" };

  virtual void layout(Rectangle<int> &area) override;
  virtual void init() override;
  virtual void cancel_cleanup() override;
  virtual bool apply() override;

  void init_text_editor(TextEditor &te, String initial_contents);
};

ProgramChangeEditor *open_program_change_editor(Connection *conn, ProgramChange *pc);
