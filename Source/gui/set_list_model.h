#pragma once

class SetListModel : public ListBoxModel {
public:
  SetListModel(SetList *set_list) : _set_list(set_list) {}

  SetList *set_list() { return _set_list; }

  int getNumRows() override { return _set_list->songs().size(); }

  void paintListBoxItem(int row, Graphics &g, int width, int height, bool rowIsSelected) override {
    if (rowIsSelected)
      g.fillAll(Colours::lightblue);

    g.setColour(LookAndFeel::getDefaultLookAndFeel().findColour(Label::textColourId));
    g.setFont((float)height * 0.7f);

    g.drawText(_set_list->songs()[row]->name(),
               5, 0, width, height,
               Justification::centredLeft, true);
  }

  // Returns a var containaing an array of int row number vars.
  var getDragSourceDescription(const SparseSet<int>& selected_rows) override {
    Array<var> row_nums;
    for (int i = 0; i < selected_rows.size(); ++i)
      row_nums.add(var(selected_rows[i]));
    return var(row_nums);
  }

protected:
  SetList * _set_list = KeyMaster_instance()->all_songs();
};
