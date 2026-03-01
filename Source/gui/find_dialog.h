#pragma once

#include <JuceHeader.h>

class FindDialog : public Component {
public:
  enum Type { FindSong, FindSetList };

  FindDialog(Type type);
  ~FindDialog() override {}

  int width();
  int height();

  void resized() override;

private:
  Type _type;

  Label _regex_label { {}, "Name (regex)" };
  TextEditor _regex;

  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };

  void ok();
  void cancel();
};

void open_find_dialog(FindDialog::Type type);
