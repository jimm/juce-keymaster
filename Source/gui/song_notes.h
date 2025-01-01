#pragma once

#include <JuceHeader.h>

class SongNotes : public TextEditor, public ActionListener, public TextEditor::Listener {
public:
  SongNotes() {
    setFont(FontOptions(18.0f));
    setMultiLine(true);
    setTabKeyUsedAsCharacter(true);
    setReturnKeyStartsNewLine(true);
    addListener(this);
  }

  virtual void textEditorTextChanged(TextEditor &) override {
    Song *song = KeyMaster_instance()->cursor()->song();
    if (song)
      song->set_notes(getText());
  }

  void update_contents() {
    Song *song = KeyMaster_instance()->cursor()->song();
    setText(song ? song->notes() : "", NotificationType::dontSendNotification);
    repaint();
  }

  void actionListenerCallback(const String &message) override {
    if (message == "moved")
      update_contents();
  }
};
