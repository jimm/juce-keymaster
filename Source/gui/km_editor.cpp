#include "km_editor.h"

KmEditor::KmEditor(bool is_new)
  : _is_new(is_new)
{
}

void KmEditor::init() {
  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };
  _apply.onClick = [this] { apply(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);
  addAndMakeVisible(_apply);

  setSize(width(), height());
}

void KmEditor::resized() {
  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);
  layout(area);
}

void KmEditor::layout(Rectangle<int> &area) {
  area.removeFromTop(BETWEEN_ROW_SPACE);

  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _ok.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _apply.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _cancel.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
}

void KmEditor::ok() {
  if (apply())
    static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void KmEditor::cancel() {
  if (_is_new)
    cancel_cleanup();
  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void KmEditor::display_errors(String thing_name) {
  String message = "The following";
  message << String((error_messages.size() == 1) ? "error prevents" : "errors prevent");
  message << " the " << thing_name.toLowerCase() << " from being saved:\n";
  for (auto err : error_messages) {
    message << "\n- ";
    message << err;
  }
  AlertWindow::showMessageBoxAsync(
    MessageBoxIconType::WarningIcon, thing_name << " Edit Errors", message, "OK", this
  );
}
