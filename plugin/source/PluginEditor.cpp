#include "DynamicsDoctor/PluginProcessor.h" // Provides DynamicsDoctorProcessor
#include "DynamicsDoctor/PluginEditor.h"    // Provides DynamicsDoctorEditor class declaration
#include "DynamicsDoctor/Constants.h"       // Provides Palette, DynamicsStatus, presets, ParameterIDs, helpers

//==============================================================================
DynamicsDoctorEditor::DynamicsDoctorEditor (DynamicsDoctorProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processorRef (p), valueTreeState (vts)
{
    // Set up the title label
    titleLabel.setText("DynamicsDoctor", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, Palette::Foreground);
    addAndMakeVisible(titleLabel);

    // Initialize and configure UI components
    addAndMakeVisible (trafficLight);
    
    // Configure status label
    statusLabel.setFont (juce::Font (16.0f));
    statusLabel.setJustificationType (juce::Justification::centred);
    statusLabel.setText ("Initializing...", juce::dontSendNotification);
    addAndMakeVisible (statusLabel);
    
    // Configure preset selector and label
    presetLabel.setFont (juce::FontOptions(14.0f));
    presetLabel.setJustificationType (juce::Justification::centredRight);
    presetLabel.attachToComponent(&presetSelector, true);
    addAndMakeVisible (presetLabel);
    
    presetSelector.setTooltip ("Select the dynamic range reference standard");
    presetSelector.clear();
    for (int i = 0; i < presets.size(); ++i)
    {
        presetSelector.addItem (presets[i].label, i + 1);
    }
    addAndMakeVisible (presetSelector);
    presetSelector.addListener (this);

    // Connect preset selector to parameter
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        valueTreeState, ParameterIDs::preset.getParamID(), presetSelector);

    // Configure measurement value labels
    peakValueLabel.setFont (juce::FontOptions(12.0f));
    peakValueLabel.setJustificationType (juce::Justification::centred);
    peakValueLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.7f));
    addAndMakeVisible (peakValueLabel);

    lraValueLabel.setFont (juce::FontOptions(12.0f));
    lraValueLabel.setJustificationType (juce::Justification::centred);
    lraValueLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.7f));
    addAndMakeVisible (lraValueLabel);

    presetInfoLabel.setFont (juce::FontOptions(12.0f));
    presetInfoLabel.setJustificationType (juce::Justification::centred);
    presetInfoLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.7f));
    addAndMakeVisible (presetInfoLabel);

    // Configure LRA reset button
    resetLraButton.setTooltip("Reset the Loudness Range (LRA) measurement history");
    addAndMakeVisible(resetLraButton);
    
    // Set up LRA reset button callback
    resetLraButton.onClick = [this]()
    {
        auto* resetParamRaw = valueTreeState.getParameter(ParameterIDs::resetLra.getParamID());
        if (resetParamRaw != nullptr)
        {
            resetParamRaw->setValueNotifyingHost(1.0f);
        }
        else
        {
            jassertfalse; // Reset parameter not found
        }
    };
    
    // Configure version label
    versionLabel.setText ("Build: " + juce::String(__DATE__) + " " + juce::String(__TIME__), juce::dontSendNotification);
    versionLabel.setFont(juce::FontOptions (10.0f));
    versionLabel.setJustificationType(juce::Justification::bottomRight);
    versionLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.6f));
    addAndMakeVisible(versionLabel);

    // Set editor size and start UI update timer
    setSize (400, 500);
    startTimerHz (30);
    updateUIStatus();
}

DynamicsDoctorEditor::~DynamicsDoctorEditor()
{
    // Stop the timer safely
    stopTimer();

    // Remove listeners explicitly added
    presetSelector.removeListener(this);
    resetLraButton.removeListener(this);
    
    // Attachments (`presetAttachment`) are automatically cleaned up
    // by their std::unique_ptr destructors.
}

//==============================================================================
void DynamicsDoctorEditor::paint (juce::Graphics& g)
{
    // Draw background
    g.fillAll(Palette::Background);

    // Draw UI sections
    auto bounds = getLocalBounds();
    auto topAreaHeight = bounds.getHeight() * 0.45f;
    auto lightArea = bounds.removeFromTop(static_cast<int>(topAreaHeight)).reduced(20);
    
    // Draw traffic light container
    g.setColour(Palette::Foreground.withAlpha(0.1f));
    g.drawRoundedRectangle(lightArea.toFloat(), 5.0f, 2.0f);
    
    // Draw section separator
    g.setColour(Palette::Foreground.withAlpha(0.2f));
    g.drawLine(static_cast<float>(bounds.getX()), topAreaHeight, static_cast<float>(bounds.getRight()), topAreaHeight, 1.0f);
}

void DynamicsDoctorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area (top 10% of the window)
    auto titleArea = bounds.removeFromTop(bounds.getHeight() * 0.1f);
    titleLabel.setBounds(titleArea);
    
    // Traffic light area (next 45% of the window)
    auto topAreaHeight = bounds.getHeight() * 0.45f;
    auto lightArea = bounds.removeFromTop(static_cast<int>(topAreaHeight)).reduced(20);
    
    // Calculate the height for the traffic light component
    auto trafficLightHeight = lightArea.getHeight() * 0.7f;
    auto trafficLightWidth = trafficLightHeight * 0.4f; // Maintain aspect ratio
    auto trafficLightX = lightArea.getCentreX() - (trafficLightWidth / 2);
    auto trafficLightY = lightArea.getCentreY() - (trafficLightHeight / 2);
    
    trafficLight.setBounds(static_cast<int>(trafficLightX),
                          static_cast<int>(trafficLightY),
                          static_cast<int>(trafficLightWidth),
                          static_cast<int>(trafficLightHeight));
    
    // Status label below traffic light
    auto statusArea = lightArea.removeFromBottom(lightArea.getHeight() * 0.2f);
    statusLabel.setBounds(statusArea);
    
    // Control area (remaining 45% of the window)
    auto controlArea = bounds.reduced(20);
    
    // Peak and LRA labels
    auto measurementArea = controlArea.removeFromTop(controlArea.getHeight() * 0.2f);
    peakValueLabel.setBounds(measurementArea.removeFromTop(measurementArea.getHeight() / 2));
    lraValueLabel.setBounds(measurementArea);
    
    // Preset controls
    auto presetArea = controlArea.removeFromTop(controlArea.getHeight() * 0.2f);
    auto presetLabelWidth = 60;
    presetLabel.setBounds(presetArea.removeFromLeft(presetLabelWidth));
    presetSelector.setBounds(presetArea);
    
    // Preset info
    presetInfoLabel.setBounds(controlArea.removeFromTop(controlArea.getHeight() * 0.15f));
    
    // Reset LRA button
    auto buttonArea = controlArea.removeFromTop(controlArea.getHeight() * 0.15f);
    resetLraButton.setBounds(buttonArea.withSizeKeepingCentre(100, 30));
    
    // Position version label
    versionLabel.setBounds(getLocalBounds().reduced(20).removeFromBottom(15).removeFromRight(100));
}

//==============================================================================
void DynamicsDoctorEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &resetLraButton)
    {
        updateUIStatus();
    }
}

//==============================================================================
// Listener Callbacks (Optional but useful for immediate feedback)

void DynamicsDoctorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetSelector)
    {
        updateUIStatus();
    }
}

//==============================================================================
// Timer Callback (Essential for polling processor analysis results)
// In PluginEditor.cpp
void DynamicsDoctorEditor::timerCallback()
{
    // Update UI status
    updateUIStatus();
    
    // Handle flashing states
    auto* processor = dynamic_cast<DynamicsDoctorProcessor*>(getAudioProcessor());
    if (processor != nullptr)
    {
        const bool isCurrentlyMeasuring = (processor->getCurrentStatus() == DynamicsStatus::Measuring);
        const bool isAwaitingAudio = (processor->getCurrentStatus() == DynamicsStatus::AwaitingAudio);
        
        if (isCurrentlyMeasuring || isAwaitingAudio)
        {
            // Handle measuring or awaiting audio state flashing
            flashTimer += 1.0 / 30.0;
            if (flashTimer >= FLASH_INTERVAL)
            {
                flashTimer = 0.0;
                isFlashingStateOn = !isFlashingStateOn;
                repaint();
            }
        }
        else
        {
            isFlashingStateOn = false;
            flashTimer = 0.0;
        }
    }
}

//==============================================================================
// Private Helper Function to Update UI Elements
void DynamicsDoctorEditor::updateUIStatus()
{
    auto* processor = dynamic_cast<DynamicsDoctorProcessor*>(getAudioProcessor());
    if (processor == nullptr) return;

    const bool isBypassed = processor->isCurrentlyBypassed();
    const bool isCurrentlyMeasuring = (processor->getCurrentStatus() == DynamicsStatus::Measuring);
    const bool isAwaitingAudio = (processor->getCurrentStatus() == DynamicsStatus::AwaitingAudio);

    // Update status indicators
    trafficLight.setStatus(processor->getCurrentStatus());
    statusLabel.setText(getStatusMessage(processor->getCurrentStatus()), juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, getStatusColour(processor->getCurrentStatus()));

    // Handle bypassed state
    if (isBypassed)
    {
        // Hide measurement displays
        peakValueLabel.setVisible(false);
        lraValueLabel.setVisible(false);
        presetInfoLabel.setVisible(false);

        // Disable controls
        presetSelector.setEnabled(false);
        presetLabel.setEnabled(false);
        resetLraButton.setEnabled(false);
        presetLabel.setColour(juce::Label::textColourId, Palette::DisabledText);
        lraValueLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.7f));
    }
    // Handle awaiting audio state
    else if (isAwaitingAudio)
    {
        // Update peak measurement
        if (auto* peakParamPtr = valueTreeState.getRawParameterValue(ParameterIDs::peak.getParamID()))
        {
            peakValueLabel.setText("Peak: " + juce::String(peakParamPtr->load(), 1) + " dBFS", 
                                 juce::dontSendNotification);
        }
        else
        {
            peakValueLabel.setText("Peak: --- dBFS", juce::dontSendNotification);
        }

        // Update LRA display
        lraValueLabel.setText("Loudness Range (LRA): Waiting for audio...", juce::dontSendNotification);
        
        // Handle flashing animation
        if (isFlashingStateOn)
        {
            lraValueLabel.setColour(juce::Label::textColourId, Palette::Ok);
            statusLabel.setColour(juce::Label::textColourId, Palette::Ok);
        }
        else
        {
            lraValueLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.5f));
            statusLabel.setColour(juce::Label::textColourId, Palette::Ok.withAlpha(0.5f));
        }

        // Update preset information
        updatePresetInfo();

        // Enable all controls
        enableControls(true);
    }
    // Handle measuring state
    else if (isCurrentlyMeasuring)
    {
        // Update peak measurement
        if (auto* peakParamPtr = valueTreeState.getRawParameterValue(ParameterIDs::peak.getParamID()))
        {
            peakValueLabel.setText("Peak: " + juce::String(peakParamPtr->load(), 1) + " dBFS", 
                                 juce::dontSendNotification);
        }
        else
        {
            peakValueLabel.setText("Peak: --- dBFS", juce::dontSendNotification);
        }

        // Update LRA display with measuring animation
        lraValueLabel.setText("Loudness Range (LRA): Measuring...", juce::dontSendNotification);
        if (isFlashingStateOn)
        {
            lraValueLabel.setColour(juce::Label::textColourId, Palette::Reduced);
            statusLabel.setColour(juce::Label::textColourId, Palette::Reduced);
        }
        else
        {
            lraValueLabel.setColour(juce::Label::textColourId, Palette::Foreground.withAlpha(0.5f));
            statusLabel.setColour(juce::Label::textColourId, 
                                getStatusColour(DynamicsStatus::Measuring));
        }

        // Update preset information
        updatePresetInfo();

        // Enable all controls
        enableControls(true);
    }
    // Handle normal active state (Ok, Reduced, Loss)
    else
    {
        // Update measurements
        updateMeasurements();
        
        // Update preset information
        updatePresetInfo();

        // Enable all controls
        enableControls(true);
    }

    // Update bypass label
    bypassLabel.setEnabled(true);
    bypassLabel.setColour(juce::Label::textColourId, Palette::Foreground);
}

//==============================================================================
// Helper functions for updateUIStatus
void DynamicsDoctorEditor::updateMeasurements()
{
    // Update peak measurement
    if (auto* peakParamPtr = valueTreeState.getRawParameterValue(ParameterIDs::peak.getParamID()))
    {
        peakValueLabel.setText("Peak: " + juce::String(peakParamPtr->load(), 1) + " dBFS", 
                             juce::dontSendNotification);
    }
    else
    {
        peakValueLabel.setText("Peak: --- dBFS", juce::dontSendNotification);
    }
    
    // Update LRA measurement
    if (auto* lraParamPtr = valueTreeState.getRawParameterValue(ParameterIDs::lra.getParamID()))
    {
        lraValueLabel.setText("LRA: " + juce::String(lraParamPtr->load(), 1) + " LU", 
                            juce::dontSendNotification);
    }
    else
    {
        lraValueLabel.setText("Loudness Range (LRA): --- LU", juce::dontSendNotification);
    }
}

void DynamicsDoctorEditor::updatePresetInfo()
{
    int presetIndex = presetSelector.getSelectedId() - 1;
    if (presetIndex >= 0 && static_cast<size_t>(presetIndex) < presets.size())
    {
        const auto& selectedPreset = presets[presetIndex];
        juce::String infoText = selectedPreset.label + " (Target LRA: ";
        infoText += juce::String(selectedPreset.targetLraMin, 1) + " LU - ";
        infoText += juce::String(selectedPreset.targetLraMax, 1) + " LU)";
        presetInfoLabel.setText(infoText, juce::dontSendNotification);
    }
    else
    {
        presetInfoLabel.setText("Select Preset", juce::dontSendNotification);
    }
}

void DynamicsDoctorEditor::enableControls(bool enable)
{
    peakValueLabel.setVisible(enable);
    lraValueLabel.setVisible(enable);
    presetInfoLabel.setVisible(enable);
    presetSelector.setEnabled(enable);
    presetLabel.setEnabled(enable);
    resetLraButton.setEnabled(enable);
    presetLabel.setColour(juce::Label::textColourId, 
                         enable ? Palette::Foreground : Palette::DisabledText);
}

