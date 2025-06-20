#pragma once

// Include necessary JUCE module headers explicitly
#include <juce_gui_basics/juce_gui_basics.h> // Provides juce::Component, juce::Graphics, etc.

// Include project-specific headers required for definitions used in this header
#include "Constants.h" // Provides DynamicsStatus enum and potentially helper functions signatures if needed publicly

//==============================================================================
/**
 * A visual status indicator component that displays the current dynamics state
 * using a traffic light metaphor with three vertically arranged lights.
 * 
 * The component automatically updates its appearance based on the current
 * DynamicsStatus, highlighting the appropriate light and adjusting colors
 * according to the status. This provides an intuitive visual feedback for
 * the current dynamics processing state.
 */
class TrafficLightComponent : public juce::Component
{
public:
    //==============================================================================
    /** Creates a new traffic light component with initial Bypassed status. */
    TrafficLightComponent();

    /** Destructor. */
    ~TrafficLightComponent() override;

    //==============================================================================
    /** @internal */
    void paint(juce::Graphics& g) override;

    /** @internal */
    void resized() override;

    //==============================================================================
    /**
     * Updates the visual state of the traffic light to reflect the new status.
     * 
     * This will trigger a repaint of the component to show:
     * - Green light for Ok status
     * - Yellow light for Reduced status
     * - Red light for Loss status
     * - All lights dimmed for Bypassed status
     * 
     * @param newStatus The current dynamics processing status to display
     */
    void setStatus(DynamicsStatus newStatus);

private:
    //==============================================================================
    /**
     * Draws a single light in the traffic light display.
     * 
     * The light's appearance is determined by:
     * - Its position (top, middle, or bottom)
     * - Whether it represents the current status
     * - The overall component state (e.g., bypassed)
     * 
     * @param g The graphics context to draw into
     * @param bounds The rectangular area where the light should be drawn
     * @param lightTargetStatus The status this light represents (Ok, Reduced, or Loss)
     */
    void paintLight(juce::Graphics& g, 
                   const juce::Rectangle<float>& bounds, 
                   DynamicsStatus lightTargetStatus);

    /** The current dynamics processing status being displayed. */
    DynamicsStatus currentStatus = DynamicsStatus::Bypassed;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrafficLightComponent)
};
