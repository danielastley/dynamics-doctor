#pragma once

// Include necessary JUCE modules explicitly
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h> 
#include <juce_gui_extra/juce_gui_extra.h>

// Include project-specific headers
#include "PluginProcessor.h"
#include "TrafficLightComponent.h" // Include the traffic light header
#include "Constants.h"             // For DynamicsStatus enum, ParameterIDs etc.

//==============================================================================
/**
 * The main UI component for the DynamicsDoctor plugin.
 * 
 * This editor provides a user interface for:
 * - Monitoring dynamics processing status via traffic light
 * - Selecting and displaying preset configurations
 * - Viewing real-time peak and LRA measurements
 * - Controlling bypass state and LRA reset
 * 
 * The editor automatically updates its display based on the processor's state
 * and provides parameter controls that are synchronized with the audio processing.
 */
class DynamicsDoctorEditor : public juce::AudioProcessorEditor,
                             private juce::ComboBox::Listener, // Keep private
                             private juce::Button::Listener,   // Keep private
                             private juce::Timer             // Keep private
{
public:
    /**
     * Creates a new editor component.
     * 
     * @param processor Reference to the audio processor
     * @param vts Reference to the processor's parameter state manager
     */
    explicit DynamicsDoctorEditor (DynamicsDoctorProcessor& processor, juce::AudioProcessorValueTreeState& vts);

    /** Destructor. */
    ~DynamicsDoctorEditor() override;

    //==============================================================================
    /** @internal */
    void paint (juce::Graphics&) override;

    /** @internal */
    void resized() override;

private:
    //==============================================================================
    // Listener Callbacks (implementations in .cpp file)
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

    /**
     * Timer callback that updates the UI with current processor state.
     * Called periodically to refresh measurements and status display.
     */
    void timerCallback() override;

    //==============================================================================
    // Helper method to update UI elements based on processor status
    void updateUIStatus();
    
    //  HELPER METHOD DECLARATIONS
    void updateMeasurements();
    void updatePresetInfo();
    void enableControls(bool enable);
  

    // Reference to the processor object (required).
    DynamicsDoctorProcessor& processorRef;

    // Reference to the processor's parameter state manager (required).
    juce::AudioProcessorValueTreeState& valueTreeState;

    // --- UI Components ---
    TrafficLightComponent trafficLight;
    juce::Label titleLabel;
    juce::Label statusLabel       { "statusLabel", "Status:" };
    juce::ComboBox presetSelector { "presetSelector" };
    

    juce::Label presetLabel       { "presetLabel", "Preset:" };
    juce::Label bypassLabel       { "bypassLabel", "Bypass" };

    juce::Label peakValueLabel    { "peakValueLabel", "-inf dBFS" }; // Default text
    juce::Label lraValueLabel    { "lraValueLabel", "0.0 LU" }; // Changed default text for clarity
    juce::Label presetInfoLabel   { "presetInfoLabel", ""};       // Initially empty
    
    juce::TextButton resetLraButton { "Reset LRA" }; // Reset Button
    
    juce::Label versionLabel      { "versionLabel", ""};
    
    // --- Parameter Attachments ---
    // Use RAII to manage the connection between UI elements and parameters.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> resetLraButtonAttachment;
   
    /** UI animation state */
    bool isFlashingStateOn { false };
    double flashTimer { 0.0 };
    static constexpr double FLASH_INTERVAL = 0.5; // Half second flash interval

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicsDoctorEditor)
};
