#pragma once

#include <JuceHeader.h>
#include "../km/patch.h"
#include "km_editor.h"

class MessageBlock;

class PatchEditor : public KmEditor {
public:
  PatchEditor(Patch *c, bool is_new);
  virtual ~PatchEditor() {}

  virtual int width() override;
  virtual int height() override;

private:
  Patch *_patch;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _start_label { {}, "Start Message" };
  ComboBox _start;

  Label _stop_label { {}, "Stop Message" };
  ComboBox _stop;

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_message(Rectangle<int> &area, Label &label, ComboBox &combo_box);

  virtual void init() override;
  void init_message(MessageBlock *mb, Label &label, ComboBox &combo_box);
  virtual void cancel_cleanup() override;

  virtual bool apply() override;
};

// If patch is nullptr we create a new one.
PatchEditor * open_patch_editor(Patch *s);
