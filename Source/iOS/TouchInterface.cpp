#include "TouchInterface.h"

#if JUCE_IOS

namespace iOS {

//==============================================================================
TouchInterface::TouchInterface()
{
    activeTouches.reserve(10); // Support up to 10 simultaneous touches
}

TouchInterface::~TouchInterface() = default;

void TouchInterface::addListener(Listener* listener)
{
    listeners.add(listener);
}

void TouchInterface::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

void TouchInterface::processTouchEvent(const juce::MouseEvent& event)
{
    auto position = event.position;
    auto currentTime = juce::Time::getCurrentTime();

    // Add new touch
    TouchState newTouch;
    newTouch.startPosition = position;
    newTouch.currentPosition = position;
    newTouch.startTime = currentTime;
    newTouch.isActive = true;
    newTouch.touchId = activeTouches.size();

    activeTouches.push_back(newTouch);

    // Check for immediate gestures (tap detection)
    if (isDoubleTap(position, currentTime))
    {
        TouchGesture gesture;
        gesture.type = TouchGesture::DoubleTap;
        gesture.position = position;
        gesture.velocity = 0.0f;
        gesture.scale = 1.0f;
        gesture.touchCount = 1;
        gesture.timestamp = currentTime;

        listeners.call([&](Listener& l) { l.touchGestureDetected(gesture); });

        // Clear previous tap info
        lastTapTime = juce::Time();
        lastTapPosition = {};
    }
    else
    {
        lastTapTime = currentTime;
        lastTapPosition = position;
    }
}

void TouchInterface::processTouchMove(const juce::MouseEvent& event)
{
    auto position = event.position;

    // Update active touches
    for (auto& touch : activeTouches)
    {
        if (touch.isActive)
        {
            touch.currentPosition = position;
            break; // For simplicity, update the first active touch
        }
    }

    detectGestures();
}

void TouchInterface::processTouchEnd(const juce::MouseEvent& event)
{
    auto position = event.position;
    auto currentTime = juce::Time::getCurrentTime();

    // Find and deactivate the touch
    for (auto& touch : activeTouches)
    {
        if (touch.isActive)
        {
            touch.isActive = false;

            // Check for tap gesture
            auto distance = touch.startPosition.getDistanceFrom(position);
            auto duration = currentTime - touch.startTime;

            if (distance < TAP_THRESHOLD && duration.inSeconds() < LONG_PRESS_DURATION)
            {
                TouchGesture gesture;
                gesture.type = TouchGesture::Tap;
                gesture.position = position;
                gesture.velocity = 0.0f;
                gesture.scale = 1.0f;
                gesture.touchCount = 1;
                gesture.timestamp = currentTime;

                listeners.call([&](Listener& l) { l.touchGestureDetected(gesture); });
            }
            break;
        }
    }

    // Clean up inactive touches
    activeTouches.erase(
        std::remove_if(activeTouches.begin(), activeTouches.end(),
                      [](const TouchState& touch) { return !touch.isActive; }),
        activeTouches.end());
}

void TouchInterface::triggerHapticFeedback(juce::HapticFeedback::Type type)
{
    if (auto* haptics = juce::HapticFeedback::getInstance())
    {
        haptics->impact(type);
    }
}

void TouchInterface::announceForAccessibility(const juce::String& announcement)
{
    juce::AccessibilityHandler::postAnnouncement(announcement, juce::AccessibilityHandler::AnnouncementPriority::medium);
}

void TouchInterface::detectGestures()
{
    auto currentTime = juce::Time::getCurrentTime();

    for (const auto& touch : activeTouches)
    {
        if (!touch.isActive) continue;

        auto distance = touch.startPosition.getDistanceFrom(touch.currentPosition);
        auto duration = currentTime - touch.startTime;

        // Long press detection
        if (duration.inSeconds() > LONG_PRESS_DURATION && distance < TAP_THRESHOLD)
        {
            TouchGesture gesture;
            gesture.type = TouchGesture::LongPress;
            gesture.position = touch.currentPosition;
            gesture.velocity = 0.0f;
            gesture.scale = 1.0f;
            gesture.touchCount = 1;
            gesture.timestamp = currentTime;

            listeners.call([&](Listener& l) { l.touchGestureDetected(gesture); });
        }

        // Swipe detection
        if (distance > SWIPE_THRESHOLD)
        {
            TouchGesture gesture;
            gesture.type = classifySwipe(touch.startPosition, touch.currentPosition);
            gesture.position = touch.currentPosition;
            gesture.velocity = distance / duration.inSeconds();
            gesture.scale = 1.0f;
            gesture.touchCount = 1;
            gesture.timestamp = currentTime;

            listeners.call([&](Listener& l) { l.touchGestureDetected(gesture); });
        }
    }

    // Multi-touch gestures (pinch)
    if (activeTouches.size() >= 2)
    {
        float scale = calculatePinchScale(activeTouches);
        if (std::abs(scale - 1.0f) > 0.1f) // 10% threshold
        {
            TouchGesture gesture;
            gesture.type = TouchGesture::Pinch;
            gesture.position = (activeTouches[0].currentPosition + activeTouches[1].currentPosition) * 0.5f;
            gesture.velocity = 0.0f;
            gesture.scale = scale;
            gesture.touchCount = static_cast<int>(activeTouches.size());
            gesture.timestamp = currentTime;

            listeners.call([&](Listener& l) { l.touchGestureDetected(gesture); });
        }
    }
}

bool TouchInterface::isDoubleTap(const juce::Point<float>& position, juce::Time time) const
{
    if (lastTapTime == juce::Time()) return false;

    auto timeDiff = time - lastTapTime;
    auto distance = lastTapPosition.getDistanceFrom(position);

    return timeDiff.inSeconds() < DOUBLE_TAP_INTERVAL && distance < TAP_THRESHOLD;
}

TouchInterface::TouchGesture::Type TouchInterface::classifySwipe(const juce::Point<float>& start,
                                                                const juce::Point<float>& end) const
{
    auto delta = end - start;

    if (std::abs(delta.x) > std::abs(delta.y))
    {
        return TouchGesture::Swipe; // Horizontal swipe
    }
    else
    {
        return TouchGesture::Swipe; // Vertical swipe
    }
}

float TouchInterface::calculatePinchScale(const std::vector<TouchState>& touches) const
{
    if (touches.size() < 2) return 1.0f;

    auto startDistance = touches[0].startPosition.getDistanceFrom(touches[1].startPosition);
    auto currentDistance = touches[0].currentPosition.getDistanceFrom(touches[1].currentPosition);

    if (startDistance < 1.0f) return 1.0f; // Avoid division by zero

    return currentDistance / startDistance;
}

//==============================================================================
TouchOptimizedComponent::TouchOptimizedComponent()
{
    touchInterface.addListener(this);
}

TouchOptimizedComponent::~TouchOptimizedComponent()
{
    touchInterface.removeListener(this);
}

void TouchOptimizedComponent::mouseDown(const juce::MouseEvent& event)
{
    touchInterface.processTouchEvent(event);
    showTouchFeedback(event.position);
    Component::mouseDown(event);
}

void TouchOptimizedComponent::mouseDrag(const juce::MouseEvent& event)
{
    touchInterface.processTouchMove(event);
    Component::mouseDrag(event);
}

void TouchOptimizedComponent::mouseUp(const juce::MouseEvent& event)
{
    touchInterface.processTouchEnd(event);
    hideTouchFeedback();
    Component::mouseUp(event);
}

void TouchOptimizedComponent::touchGestureDetected(const TouchInterface::TouchGesture& gesture)
{
    switch (gesture.type)
    {
        case TouchInterface::TouchGesture::Tap:
            handleTap(gesture.position);
            break;
        case TouchInterface::TouchGesture::DoubleTap:
            handleDoubleTap(gesture.position);
            break;
        case TouchInterface::TouchGesture::LongPress:
            handleLongPress(gesture.position);
            break;
        case TouchInterface::TouchGesture::Pinch:
            handlePinch(gesture.scale, gesture.position);
            break;
        case TouchInterface::TouchGesture::Pan:
            handlePan(gesture.position);
            break;
        case TouchInterface::TouchGesture::Swipe:
            handleSwipe(gesture.position);
            break;
    }
}

void TouchOptimizedComponent::showTouchFeedback(const juce::Point<float>& position)
{
    // Create visual feedback for touch
    if (!touchFeedbackComponent)
    {
        touchFeedbackComponent = std::make_unique<juce::Component>();
        touchFeedbackComponent->setSize(44, 44); // 44pt minimum touch target
        addAndMakeVisible(*touchFeedbackComponent);
    }

    touchFeedbackComponent->setCentrePosition(position.toInt());
    touchFeedbackComponent->setAlpha(0.3f);
    touchFeedbackComponent->toFront(false);
}

void TouchOptimizedComponent::hideTouchFeedback()
{
    if (touchFeedbackComponent)
    {
        touchFeedbackComponent->setVisible(false);
    }
}

int TouchOptimizedComponent::getTouchFriendlySize(int baseSize) const
{
    return std::max(baseSize, 44); // iOS minimum touch target: 44pt
}

juce::Rectangle<int> TouchOptimizedComponent::getTouchFriendlyBounds(const juce::Rectangle<int>& baseBounds) const
{
    auto minSize = 44;
    auto width = std::max(baseBounds.getWidth(), minSize);
    auto height = std::max(baseBounds.getHeight(), minSize);

    return baseBounds.withSizeKeepingCentre(width, height);
}

//==============================================================================
TouchSlider::TouchSlider()
{
    setSliderStyle(juce::Slider::LinearVertical);
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);

    // Touch-friendly styling
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a90e2));
    setColour(juce::Slider::trackColourId, juce::Colour(0xff2d3748));
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a202c));
}

void TouchSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Draw track
    auto trackBounds = bounds.reduced(20, 10);
    g.setColour(findColour(juce::Slider::trackColourId));
    g.fillRoundedRectangle(trackBounds, 4.0f);

    // Draw thumb - larger for touch
    auto thumbY = juce::jmap(getValue(), getMinimum(), getMaximum(),
                            trackBounds.getBottom() - 20, trackBounds.getY() + 20);
    auto thumbBounds = juce::Rectangle<float>(trackBounds.getX() - 10, thumbY - 20,
                                             trackBounds.getWidth() + 20, 40);

    drawTouchFriendlyThumb(g, thumbBounds);

    // Value text
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    auto textBounds = bounds.removeFromBottom(30);
    g.drawText(getTextFromValue(getValue()), textBounds, juce::Justification::centred);
}

void TouchSlider::mouseDown(const juce::MouseEvent& event)
{
    triggerHapticFeedbackIfEnabled();
    isDragging = true;
    lastDragPosition = event.position;
    juce::Slider::mouseDown(event);
}

void TouchSlider::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        auto delta = event.position - lastDragPosition;
        auto sensitivity = touchSensitivity * 0.01f;

        // Apply touch sensitivity
        auto scaledDelta = delta * sensitivity;

        // Create modified event with scaled movement
        auto modifiedEvent = event.withNewPosition(lastDragPosition + scaledDelta);
        juce::Slider::mouseDrag(modifiedEvent);

        lastDragPosition = event.position;
    }
}

void TouchSlider::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
    juce::Slider::mouseUp(event);
}

void TouchSlider::setTouchSensitivity(float sensitivity)
{
    touchSensitivity = juce::jlimit(0.1f, 5.0f, sensitivity);
}

void TouchSlider::enableHapticFeedback(bool enabled)
{
    hapticFeedbackEnabled = enabled;
}

void TouchSlider::triggerHapticFeedbackIfEnabled()
{
    if (hapticFeedbackEnabled)
    {
        if (auto* haptics = juce::HapticFeedback::getInstance())
        {
            haptics->impact(juce::HapticFeedback::light);
        }
    }
}

void TouchSlider::drawTouchFriendlyThumb(juce::Graphics& g, const juce::Rectangle<float>& thumbBounds)
{
    g.setColour(findColour(juce::Slider::thumbColourId));
    g.fillRoundedRectangle(thumbBounds, 20.0f);

    // Add highlight for better visibility
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.fillRoundedRectangle(thumbBounds.reduced(2), 18.0f);
}

//==============================================================================
TouchButton::TouchButton(const juce::String& buttonText)
    : juce::TextButton(buttonText)
    , touchHighlightColour(juce::Colour(0xff4a90e2))
{
    // Ensure minimum touch target size
    setSize(std::max(getWidth(), 44), std::max(getHeight(), 44));
}

void TouchButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    auto bgColour = isPressed ? touchHighlightColour : findColour(juce::TextButton::buttonColourId);
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(findColour(juce::TextButton::buttonOnColourId));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);

    // Text
    g.setColour(findColour(juce::TextButton::textColourOffId));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText(getButtonText(), bounds, juce::Justification::centred);
}

void TouchButton::mouseDown(const juce::MouseEvent& event)
{
    isPressed = true;
    triggerHapticFeedbackIfEnabled();
    repaint();
    juce::TextButton::mouseDown(event);
}

void TouchButton::mouseUp(const juce::MouseEvent& event)
{
    isPressed = false;
    repaint();
    juce::TextButton::mouseUp(event);
}

void TouchButton::setTouchHighlightColour(juce::Colour colour)
{
    touchHighlightColour = colour;
}

void TouchButton::enableHapticFeedback(bool enabled)
{
    hapticFeedbackEnabled = enabled;
}

void TouchButton::triggerHapticFeedbackIfEnabled()
{
    if (hapticFeedbackEnabled)
    {
        if (auto* haptics = juce::HapticFeedback::getInstance())
        {
            haptics->impact(juce::HapticFeedback::medium);
        }
    }
}

} // namespace iOS

#endif // JUCE_IOS
