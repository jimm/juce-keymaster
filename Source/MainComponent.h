#pragma once

#include <JuceHeader.h>

class KeyMaster;

//==============================================================================
/*
  This component lives inside our window, and this is where you should put all
  your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
  //==============================================================================
  MainComponent(juce::ApplicationProperties &app_properties);
  ~MainComponent() override;

  //==============================================================================
  void paint(juce::Graphics& g) override;
  void resized() override;

private:
  //==============================================================================

  KeyMaster *km;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
