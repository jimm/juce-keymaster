#include "MainComponent.h"
#include "km/keymaster.h"

//==============================================================================
MainComponent::MainComponent(juce::ApplicationProperties &props)
{
  km = create_KeyMaster();
  km->start();

  // Make sure you set the size of the component after
  // you add any child components.
  auto settings = props.getUserSettings();
  auto width = settings->getIntValue("window.width", 800);
  auto height = settings->getIntValue("window.height", 600);
  setSize(width, height);
}

MainComponent::~MainComponent()
{
  delete km;
}


//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
  // (Our component is opaque, so we must completely fill the background with a solid colour)
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.setColour(juce::Colours::white);
  g.drawText(km->cursor()->patch()->name(), getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
  // This is called when the MainContentComponent is resized.
  // If you add any child components, this is where you should
  // update their positions.
}
