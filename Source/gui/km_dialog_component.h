#pragma once

#include <JuceHeader.h>

#define UNSELECTED (-1)

#define SPACE 12
#define BETWEEN_ROW_SPACE 20
#define LABEL_HEIGHT 16
#define BUTTON_HEIGHT 35
#define BUTTON_WIDTH 80
#define DATA_ROW_HEIGHT 20

class KmDialogComponent : public Component, public ActionBroadcaster {
public:
  KmDialogComponent(bool is_new);
  virtual ~KmDialogComponent() {}

  virtual int width() { return SPACE * 2; }
  virtual int height() { return SPACE * 2 + BETWEEN_ROW_SPACE + BUTTON_HEIGHT; }

  void resized() override;

protected:
  bool _is_new;
  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };
  TextButton _apply { "Apply" };
  Array<String> error_messages;

  // call this layout _after_ yours, not before
  virtual void layout(Rectangle<int> &area);

  virtual void init();
  virtual void cancel_cleanup() {}

  void ok();
  void cancel();
  virtual bool apply() = 0;

  void add_error_message(String message) { error_messages.add(message); }
  void add_error_message(const char *message) { error_messages.add(String(message)); }
  bool has_errors() { return !error_messages.isEmpty(); }
  void display_errors(String thing_name);
  void display_errors(const char *thing_name) { display_errors(String(thing_name)); }
};
