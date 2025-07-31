/*
  ==============================================================================

    Main.cpp
    Created: JUCE Audio Tutorial Series
    Author:  JUCE Tutorial Team

    This file contains the main application entry point for the JUCE Audio
    Tutorial Series. It demonstrates the fundamental structure of a JUCE
    application and serves as the foundation for all tutorial examples.

    Key Concepts Demonstrated:
    - JUCE Application lifecycle management
    - Window creation and management
    - Component ownership and memory management
    - Cross-platform application structure

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
/**
 * Main Application Class
 *
 * This class represents the entire JUCE application and manages its lifecycle.
 * It inherits from juce::JUCEApplication which provides the framework for
 * cross-platform application management.
 *
 * The application follows the standard JUCE pattern:
 * 1. initialise() - Called when the app starts
 * 2. shutdown() - Called when the app is closing
 * 3. systemRequestedQuit() - Called when system wants to quit the app
 */
class MainApp : public juce::JUCEApplication
{
public:
    //==============================================================================
    /**
     * Get Application Name
     *
     * Returns the name of the application as it appears in the system.
     * This name is used for:
     * - Window titles
     * - System menus
     * - Application identification
     *
     * @return The application name as a JUCE String
     */
    const juce::String getApplicationName() override { return "JUCE Audio Tutorial"; }

    /**
     * Get Application Version
     *
     * Returns the current version of the application.
     * This is used for:
     * - About dialogs
     * - System information
     * - Update checking
     *
     * @return The version string in semantic versioning format
     */
    const juce::String getApplicationVersion() override { return "1.0.0"; }

    /**
     * Application Initialization
     *
     * This method is called when the application starts up. It's where you
     * should create your main window and initialize any global resources.
     *
     * The method is guaranteed to be called on the main thread and should
     * complete quickly to avoid blocking the application startup.
     *
     * @param commandLine Command line arguments passed to the application
     */
    void initialise(const juce::String& commandLine) override
    {
        // Ignore unused parameter warning
        juce::ignoreUnused(commandLine);

        // Create the main application window
        // We use std::unique_ptr for automatic memory management
        mainWindow.reset(new MainWindow("JUCE Audio Tutorial", new MainComponent(), *this));

        // At this point, the window is created and visible
        // Any additional initialization can be done here
    }

    /**
     * Application Shutdown
     *
     * This method is called when the application is shutting down.
     * Use this to clean up resources, save settings, and perform
     * any necessary cleanup operations.
     *
     * This method is guaranteed to be called on the main thread.
     */
    void shutdown() override
    {
        // Clean up the main window
        // Setting the unique_ptr to nullptr automatically deletes the window
        mainWindow = nullptr;

        // Any additional cleanup can be done here:
        // - Save application settings
        // - Close audio devices
        // - Clean up global resources
        // - Save user data
    }

private:
    //==============================================================================
    /**
     * Main Window Class
     *
     * This class represents the main application window. It inherits from
     * juce::DocumentWindow which provides standard window functionality
     * including title bar, resize handles, and close button.
     *
     * The window acts as a container for the main application content
     * and handles window-specific events and behavior.
     */
    class MainWindow : public juce::DocumentWindow
    {
    public:
        /**
         * Main Window Constructor
         *
         * Creates and configures the main application window.
         *
         * @param name The window title
         * @param c The main content component (takes ownership)
         * @param app Reference to the parent application
         */
        MainWindow(juce::String name, juce::Component* c, JUCEApplication& app)
            : DocumentWindow(name,                           // Window title
                           juce::Desktop::getInstance()      // Background color from system
                               .getDefaultLookAndFeel()
                               .findColour(juce::ResizableWindow::backgroundColourId),
                           DocumentWindow::allButtons),      // Show all window buttons
              owner(app)                                     // Store reference to parent app
        {
            // Configure window properties
            setUsingNativeTitleBar(true);        // Use system-native title bar
            setContentOwned(c, true);            // Take ownership of content component

            // Set window size and position
            #if JUCE_IOS || JUCE_ANDROID
                setFullScreen(true);             // Full screen on mobile platforms
            #else
                setResizable(true, true);        // Allow resizing on desktop
                centreWithSize(getWidth(), getHeight());  // Center on screen
            #endif

            setVisible(true);                    // Make window visible
        }

        /**
         * Close Button Pressed Handler
         *
         * This method is called when the user clicks the window's close button.
         * We delegate to the application's quit mechanism to ensure proper
         * shutdown sequence.
         */
        void closeButtonPressed() override
        {
            // Request application shutdown through proper channels
            // This allows the application to perform cleanup before quitting
            owner.systemRequestedQuit();
        }

    private:
        /**
         * Reference to Parent Application
         *
         * We store a reference to the parent application so we can
         * communicate with it when window events occur (like close button).
         */
        JUCEApplication& owner;
    };

    //==============================================================================
    /**
     * Main Window Instance
     *
     * We use std::unique_ptr for automatic memory management.
     * When the application shuts down, setting this to nullptr
     * will automatically delete the window and all its contents.
     */
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
/**
 * Application Entry Point Macro
 *
 * This macro creates the main() function and handles all the platform-specific
 * application startup code. It creates an instance of MainApp and runs the
 * JUCE message loop.
 *
 * The macro handles:
 * - Platform-specific initialization
 * - Message loop management
 * - Proper application shutdown
 * - Exception handling
 * - Memory management
 */
START_JUCE_APPLICATION(MainApp)
