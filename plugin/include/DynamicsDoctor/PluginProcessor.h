#pragma once

// Core JUCE modules
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>

// Project-specific headers
#include "Constants.h"
#include "LoudnessMeter.h"

//==============================================================================
/**
 * Main audio processing class for the DynamicsDoctor plugin.
 * 
 * This class handles:
 * - Real-time audio processing and loudness analysis
 * - Parameter management and state persistence
 * - Communication with the plugin editor
 * - Loudness Range (LRA) measurement and status reporting
 * 
 * The processor maintains a state machine that tracks the current dynamics status
 * (Ok, Reduced, Loss, Measuring, Bypassed) based on the measured LRA values
 * and the selected preset's thresholds.
 */
class DynamicsDoctorProcessor : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    DynamicsDoctorProcessor();
    ~DynamicsDoctorProcessor() override;

    //==============================================================================
    /** Audio processing callbacks */
    void prepareToPlay (double newSampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    /** Plugin editor management */
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    /** Plugin metadata and capabilities */
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    /** Program/preset management */
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    /** State persistence */
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Public interface for the editor */
    juce::AudioProcessorValueTreeState& getValueTreeState();
    DynamicsStatus getCurrentStatus() const;
    float getReportedLRA() const;
    
    // <<< ADD THESE NEW PUBLIC GETTERS >>>
    bool isCurrentlyBypassed() const;
    
    /** Parameter change callback from the value tree state */
    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    //==============================================================================
    /** Parameter management */
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /** Real-time parameter access - atomic for thread safety */
    std::atomic<float>* bypassParam = nullptr;      // Bypass state
    std::atomic<float>* presetParam = nullptr;      // Selected preset index
    std::atomic<float>* peakParam = nullptr;        // Instantaneous peak level in dBFS
    std::atomic<float>* lraParam = nullptr;         // Long-term loudness range in LU
    juce::AudioParameterBool* resetLraParamObject = nullptr;  // LRA reset trigger
    
    /** Loudness analysis engine */
    LoudnessMeter loudnessMeter;
    
    /** Timing and state management */
    double internalSampleRate = 0.0;                // Current sample rate
    int samplesUntilLraUpdate = 0;                  // Counter for LRA update timing
    std::atomic<int> samplesProcessedSinceReset {0}; // Samples since last LRA reset
    
    /** Analysis results - atomic for thread-safe editor access */
    std::atomic<float> currentPeak { ParameterDefaults::peak };  // Current peak level
    std::atomic<float> currentGlobalLRA { 0.0f };                // Current LRA value
    std::atomic<DynamicsStatus> currentStatus { DynamicsStatus::Measuring }; // Current state
    
    /** Audio state monitoring */
    std::atomic<double> timeSinceLastAudio { 0.0 };              // Time since last audio signal
    std::atomic<bool> waitingForNextAudio { false };             // Flag to indicate we're waiting for next audio signal
    std::atomic<bool> isInitialMeasuringPhase { true };  // Flag to track initial measuring phase
    static constexpr double LRA_MEASURING_DURATION_SECONDS = 15.0;    // Duration for LRA measurement (15 seconds)
    static constexpr double AUDIO_TIMEOUT = 300.0;                    // 5 minutes timeout for no audio
    
    /** Internal processing methods */
    void handleResetLRA();                         // Reset LRA measurement
    void updateStatusBasedOnLRA(float measuredLRA); // Update status based on LRA thresholds
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicsDoctorProcessor)
};
