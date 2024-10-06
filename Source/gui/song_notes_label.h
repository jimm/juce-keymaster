#pragma

class SongNotesLabel : public Label, public ActionListener {
public:
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
