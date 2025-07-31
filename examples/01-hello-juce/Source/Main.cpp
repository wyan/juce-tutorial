/*
  ==============================================================================

    Hello JUCE - Basic Application Example

    This file demonstrates the fundamental structure of a JUCE application.
    It shows how to create a simple window with basic graphics and interaction.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
/**
 * Main Window Class
 *
 * This class manages the application's main window. It inherits from
 * DocumentWindow which provides standard window functionality like
 * title bar, resize handles, and close button.
 */
class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name)
        : DocumentWindow(name,
                        juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                   .findColour(juce::ResizableWindow::backgroundColourId),
                        DocumentWindow::allButtons)
    {
        // Make the window visible and set its properties
        setUsingNativeTitleBar(true);           // Use system title bar
        setContentOwned(new MainComponent(), true);  // Set main component as content

        #if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);                // Full screen on mobile
        #else
            setResizable(true, true);           // Allow resizing on desktop
            centreWithSize(getWidth(), getHeight());  // Center the window
        #endif

        setVisible(true);                       // Make window visible
    }

    /**
     * Handle the close button click
     * This is called when user clicks the window's close button
     */
    void closeButtonPressed() override
    {
        // This will trigger the application to quit
        JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

//==============================================================================
/**
 * Main Application Class
 *
 * This class represents the entire application. It manages the application
 * lifecycle and creates/destroys the main window.
 */
class HelloJUCEApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    HelloJUCEApplication() {}

    /**
     * Return the application name
     * This appears in the window title and system menus
     */
    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }

    /**
     * Return the application version
     * Used for about dialogs and system information
     */
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }

    /**
     * Specify if multiple instances are allowed
     * Return false to prevent multiple instances of the app
     */
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    /**
     * Application Initialization
     *
     * This is called when the application starts up. Use this to create
     * your main window and initialize any global resources.
     *
     * @param commandLine Command line arguments passed to the application
     */
    void initialise(const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        // Create the main window
        mainWindow.reset(new MainWindow(getApplicationName()));

        // You could add additional initialization here:
        // - Load settings
        // - Initialize audio devices
        // - Set up global state
        // - Parse command line arguments
    }

    /**
     * Application Shutdown
     *
     * This is called when the application is being shut down. Use this to
     * clean up resources, save settings, and perform final cleanup.
     */
    void shutdown() override
    {
        // Add your application's shutdown code here..

        // Clean up the main window
        mainWindow = nullptr; // (deletes our window)

        // You could add additional cleanup here:
        // - Save application settings
        // - Close audio devices
        // - Clean up global resources
        // - Save user data
    }

    //==============================================================================
    /**
     * System Requested Quit
     *
     * This is called when the system asks the application to quit
     * (e.g., user logs out, system shutdown, etc.)
     */
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    /**
     * Another Instance Started
     *
     * This is called when another instance of the app is launched while this one
     * is running. Only called if moreThanOneInstanceAllowed() returns false.
     */
    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.

        // You could use this to bring the current instance to the front,
        // or to handle the command line arguments from the new instance.
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
/**
 * Application Entry Point
 *
 * This macro creates the main() function and handles platform-specific
 * application startup. It creates an instance of HelloJUCEApplication
 * and runs the JUCE message loop.
 */
START_JUCE_APPLICATION(HelloJUCEApplication)
