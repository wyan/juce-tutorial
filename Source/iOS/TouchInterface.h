#pragma once

#include <JuceHeader.h>

#if JUCE_IOS

namespace iOS {

//==============================================================================
/**
 * Touch interface manager for iOS-specific touch optimizations
 */
class TouchInterface
{
public:
    TouchInterface();
    ~TouchInterface();

    // Touch gesture recognition
    struct TouchGesture
    {
        enum Type { Tap, DoubleTap, LongPress, Pinch, Pan, Swipe };
        Type type;
        juce::Point<float> position;
        float velocity;
        float scale;
        int touchCount;
        juce::Time timestamp;
    };

    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void touchGestureDetected(const TouchGesture& gesture) = 0;
    };

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    // Touch event processing
    void processTouchEvent(const juce::MouseEvent& event);
    void processTouchMove(const juce::MouseEvent& event);
    void processTouchEnd(const juce::MouseEvent& event);

    // Haptic feedback
    void triggerHapticFeedback(juce::HapticFeedback::Type type);

    // Accessibility
    void announceForAccessibility(const juce::String& announcement);

private:
    juce::ListenerList<Listener> listeners;

    // Gesture detection state
    struct TouchState
    {
        juce::Point<float> startPosition;
        juce::Point<float> currentPosition;
        juce::Time startTime;
        bool isActive = false;
        int touchId = -1;
    };

    std::vector<TouchState> activeTouches;
    juce::Time lastTapTime;
    juce::Point<float> lastTapPosition;

    // Gesture thresholds
    static constexpr float LONG_PRESS_DURATION = 0.5f; // seconds
    static constexpr float DOUBLE_TAP_INTERVAL = 0.3f; // seconds
    static constexpr float SWIPE_THRESHOLD = 50.0f; // pixels
    static constexpr float TAP_THRESHOLD = 10.0f; // pixels
    static constexpr float PINCH_THRESHOLD = 20.0f; // pixels

    void detectGestures();
    bool isDoubleTap(const juce::Point<float>& position, juce::Time time) const;
    TouchGesture::Type classifySwipe(const juce::Point<float>& start,
                                    const juce::Point<float>& end) const;
    float calculatePinchScale(const std::vector<TouchState>& touches) const;
};

//==============================================================================
/**
 * Touch-optimized component base class
 */
class TouchOptimizedComponent : public juce::Component,
                               public TouchInterface::Listener
{
public:
    TouchOptimizedComponent();
    ~TouchOptimizedComponent() override;

    // Component interface
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // TouchInterface::Listener
    void touchGestureDetected(const TouchInterface::TouchGesture& gesture) override;

    // Touch-specific virtual methods
    virtual void handleTap(const juce::Point<float>& position) {}
    virtual void handleDoubleTap(const juce::Point<float>& position) {}
    virtual void handleLongPress(const juce::Point<float>& position) {}
    virtual void handlePinch(float scale, const juce::Point<float>& center) {}
    virtual void handlePan(const juce::Point<float>& delta) {}
    virtual void handleSwipe(const juce::Point<float>& direction) {}

protected:
    TouchInterface touchInterface;

    // Touch state
    bool isTouchActive = false;
    juce::Point<float> touchStartPosition;

    // Visual feedback
    void showTouchFeedback(const juce::Point<float>& position);
    void hideTouchFeedback();

    // Touch-friendly sizing
    int getTouchFriendlySize(int baseSize) const;
    juce::Rectangle<int> getTouchFriendlyBounds(const juce::Rectangle<int>& baseBounds) const;

private:
    std::unique_ptr<juce::Component> touchFeedbackComponent;

    class TouchFeedbackComponent;
};

//==============================================================================
/**
 * Touch-optimized slider for iOS
 */
class TouchSlider : public juce::Slider
{
public:
    TouchSlider();
    ~TouchSlider() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Touch-specific features
    void setTouchSensitivity(float sensitivity);
    void enableHapticFeedback(bool enabled);

private:
    float touchSensitivity = 1.0f;
    bool hapticFeedbackEnabled = true;
    bool isDragging = false;
    juce::Point<float> lastDragPosition;

    void triggerHapticFeedbackIfEnabled();
    void drawTouchFriendlyThumb(juce::Graphics& g, const juce::Rectangle<float>& thumbBounds);
};

//==============================================================================
/**
 * Touch-optimized button for iOS
 */
class TouchButton : public juce::TextButton
{
public:
    TouchButton(const juce::String& buttonText = {});
    ~TouchButton() override = default;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    // Touch-specific features
    void setTouchHighlightColour(juce::Colour colour);
    void enableHapticFeedback(bool enabled);

private:
    juce::Colour touchHighlightColour;
    bool hapticFeedbackEnabled = true;
    bool isPressed = false;

    void triggerHapticFeedbackIfEnabled();
};

} // namespace iOS

#endif // JUCE_IOS
