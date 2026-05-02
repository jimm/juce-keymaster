#pragma once

#include <JuceHeader.h>
#include "../km/patch.h"
#include "../km/instrument_program_change.h"
#include "km_editor.h"

class MessageBlock;

class IpcTableModel : public TableListBoxModel {
public:
  IpcTableModel(Array<InstrumentProgramChange> &ipcs) : _ipcs(ipcs) {}

  int getNumRows() override;
  void paintRowBackground(Graphics &g, int row, int w, int h, bool sel) override;
  void paintCell(Graphics &, int, int, int, int, bool) override {}
  Component* refreshComponentForCell(int row, int col, bool sel,
                                     Component *existing) override;

private:
  Array<InstrumentProgramChange> &_ipcs;
};

class PatchEditor : public KmEditor {
public:
  PatchEditor(Patch *c, bool is_new);
  virtual ~PatchEditor() {}

  virtual int width() override;
  virtual int height() override;

  Array<InstrumentProgramChange> &edit_ipcs() { return _edit_ipcs; }

private:
  Patch *_patch;
  Array<InstrumentProgramChange> _edit_ipcs;
  IpcTableModel _ipc_model;
  TableListBox _ipc_table;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _start_label { {}, "Start Message" };
  ComboBox _start;

  Label _stop_label { {}, "Stop Message" };
  ComboBox _stop;

  Label _ipc_label { {}, "Program Changes" };
  TextButton _add_ipc { "+" };
  TextButton _del_ipc { "-" };

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_message(Rectangle<int> &area, Label &label, ComboBox &combo_box);
  void layout_ipc_table(Rectangle<int> &area);

  virtual void init() override;
  void init_message(MessageBlock *mb, Label &label, ComboBox &combo_box);
  void init_ipc_table();
  virtual void cancel_cleanup() override;

  void add_ipc();
  void del_ipc();

  virtual bool apply() override;
};

// If patch is nullptr we create a new one.
PatchEditor * open_patch_editor(Patch *s);
