#include "connection_dialog_component.h"
#include "../km/patch.h"
#include "../km/connection.h"

ConnectionDialogComponent::ConnectionDialogComponent(Patch *p, Connection *c)
  : _patch(p), _conn(c), _e_conn(*c)
{
  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };
  _apply.onClick = [this] { apply(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);
  addAndMakeVisible(_apply);

  setSize(300, 200);
}

void ConnectionDialogComponent::resized() {
  using Track = Grid::TrackInfo;
  using Fr = Grid::Fr;
  using Px = Grid::Px;

  Grid button_grid;
  button_grid.templateRows = { Track(Fr(1)) };
  button_grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  button_grid.items = { GridItem(_ok), GridItem(_cancel), GridItem(_apply) };

  button_grid.performLayout(getLocalBounds().removeFromBottom(20));
}

void ConnectionDialogComponent::ok() {
  DBG("ok");
  apply();
  // closeButtonPressed();
}

void ConnectionDialogComponent::cancel() {
  DBG("cancel");
  // TODO
  // closeButtonPressed();
}

void ConnectionDialogComponent::apply() {
  DBG("apply");
  // TODO
}
