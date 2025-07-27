#include <JuceHeader.h>
#include "MainComponent.h"

class MainApp  : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "TestJuce"; }
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    void initialise (const juce::String&) override         { mainWindow.reset (new MainWindow ("TestJuce", new MainComponent(), *this)); }
    void shutdown() override                               { mainWindow = nullptr; }

private:
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow (juce::String name, juce::Component* c, JUCEApplication& app)
            : DocumentWindow (name, juce::Colours::lightgrey, DocumentWindow::allButtons), owner(app)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (c, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }
        void closeButtonPressed() override { owner.systemRequestedQuit(); }
    private:
        JUCEApplication& owner;
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (MainApp)
