#include <JuceHeader.h>
#include "MainComponent.h"
#include "km/device_manager.h"

#define DEFAULT_WINDOW_X (-1)
#define DEFAULT_WINDOW_Y (-1)

//==============================================================================
class KeyMasterApplication  : public JUCEApplication
{
public:
  //==============================================================================
  KeyMasterApplication() {}

  const String getApplicationName() override       { return ProjectInfo::projectName; }
  const String getApplicationVersion() override    { return ProjectInfo::versionString; }
  bool moreThanOneInstanceAllowed() override             { return true; }

  //==============================================================================
  void initialise(const String& commandLine) override
    {
      // This method is where you should put your application's initialisation code.

      app_properties.setStorageParameters ([&]
        {
          PropertiesFile::Options opt;
          opt.applicationName = "KeyMaster";
          opt.commonToAllUsers = false;
          opt.doNotSave = false;
          opt.filenameSuffix = ".props";
          opt.ignoreCaseOfKeyNames = false;
          opt.storageFormat = PropertiesFile::StorageFormat::storeAsXML;
          opt.osxLibrarySubFolder = "Application Support";
          return opt;
        }());
      device_manager.update_devices();
      mainWindow.reset(new MainWindow(getApplicationName(), device_manager, app_properties));
    }

  void shutdown() override
    {
      // Add your application's shutdown code here..

      auto win = mainWindow.get();
      auto settings = app_properties.getUserSettings();
      settings->setValue("window.x", win->getX());
      settings->setValue("window.y", win->getY());
      auto comp = win->getChildComponent(0);
      settings->setValue("window.width", comp->getWidth());
      settings->setValue("window.height", comp->getHeight());
      app_properties.closeFiles();

      mainWindow = nullptr; // (deletes our window)
    }

  //==============================================================================
  void systemRequestedQuit() override
    {
      // This is called when the app is being asked to quit: you can ignore this
      // request and let the app carry on running, or call quit() to allow the app to close.
      mainWindow->check_ok_to_quit([] (bool ok) {
        if (ok)
          quit();
      });
    }

  void anotherInstanceStarted(const String& commandLine) override
    {
      // When another instance of the app is launched while this one is running,
      // this method is invoked, and the commandLine parameter tells you what
      // the other instance's command-line arguments were.

      File file(commandLine);
      mainWindow->load_document(file);
    }

  //==============================================================================
  /*
    This class implements the desktop window that contains an instance of
    our MainComponent class.
  */
  class MainWindow : public DocumentWindow
  {
  public:
    MainWindow(String name, DeviceManager &device_manager, ApplicationProperties &props)
      : DocumentWindow(name,
                       Desktop::getInstance().getDefaultLookAndFeel()
                       .findColour(ResizableWindow::backgroundColourId),
                       DocumentWindow::allButtons),
        app_properties(props)
      {
        setUsingNativeTitleBar(true);
        main_component = std::unique_ptr<MainComponent>(new MainComponent(device_manager, app_properties));
        setContentOwned(main_component.get(), true);

#if JUCE_IOS || JUCE_ANDROID
        setFullScreen(true);
#else
        setResizable(true, true);

        auto settings = app_properties.getUserSettings();
        auto x = settings->getIntValue("window.x", DEFAULT_WINDOW_X);
        auto y = settings->getIntValue("window.y", DEFAULT_WINDOW_Y);
        if (x != DEFAULT_WINDOW_X && y != DEFAULT_WINDOW_Y)
          setBounds(x, y, getWidth(), getHeight());
        else
          centreWithSize(getWidth(), getHeight());
#endif

        setVisible(true);
      }

    ~MainWindow() {
      main_component = nullptr;
    }

    void load_document(const File &file) {
      auto result = main_component->loadDocument(file);
      if (result != Result::ok()) { // old instance is still intact
        AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                                         "Error loading KeyMaster file",
                                         result.getErrorMessage(),
                                         "OK");
      }
    }

    void closeButtonPressed() override
      {
        // This is called when the user tries to close this window. Here, we'll just
        // ask the app to quit when this happens, but you can change this to do
        // whatever you need.
        JUCEApplication::getInstance()->systemRequestedQuit();
      }

    /* Note: Be careful if you override any DocumentWindow methods - the base
       class uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in your content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       subclass also calls the superclass's method.
    */

    void check_ok_to_quit(std::function<void(bool)> callback) { main_component->check_ok_to_quit(callback); }

  private:
    ApplicationProperties &app_properties;
    std::unique_ptr<MainComponent> main_component;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
  ApplicationProperties app_properties;
  DeviceManager device_manager;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(KeyMasterApplication)
