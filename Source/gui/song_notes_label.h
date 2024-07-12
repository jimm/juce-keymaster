#pragma

class SongNotesLabel : public Label, public ActionListener {
public:
  void update_contents() {
    Song *song = KeyMaster_instance()->cursor()->song();
    if (song) {
      setText(song->notes(), NotificationType::dontSendNotification);
      repaint();
    }
  }

  void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      update_contents();
    }
  }
};
