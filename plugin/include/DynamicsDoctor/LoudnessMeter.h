#pragma once

#include <juce_audio_basics/juce_audio_basics.h> // For juce::AudioBuffer
#include "ebur128.h" // <<< Crucial: Include the C library's header

//==============================================================================
/**
 * C++ wrapper for the libebur128 library.
 * 
 * Provides a simplified interface for measuring audio loudness according to
 * EBU R128 / ITU-R BS.1770 standards. Handles initialization, processing,
 * and retrieval of various loudness measurements including:
 * - Short-term loudness
 * - Momentary loudness
 * - Loudness Range (LRA)
 */
class LoudnessMeter
{
public:
    LoudnessMeter();
    ~LoudnessMeter();

    /**
     * Initializes the loudness meter for audio processing.
     * Must be called before any audio processing can occur.
     * 
     * @param sampleRate The audio sample rate in Hz
     * @param numChannels The number of audio channels (1=mono, 2=stereo)
     * @param maxSamplesPerBlock The maximum number of samples per processing block
     */
    void prepare(double sampleRate, int numChannels, int maxSamplesPerBlock);

    /**
     * Resets all internal measurements and state.
     * Call this when starting a new measurement session.
     */
    void reset();

    /**
     * Processes a block of audio samples for loudness analysis.
     * This should be called for each block of audio data.
     * 
     * @param buffer The audio buffer containing the samples to analyze
     */
    void processBlock(const juce::AudioBuffer<float>& buffer);

    /**
     * Retrieves the short-term loudness measurement.
     * This is the loudness measured over a 3-second window.
     * 
     * @return Short-term loudness in LUFS, or -144.0 if insufficient data
     */
    float getShortTermLoudness() const;

    /**
     * Retrieves the momentary loudness measurement.
     * This is the loudness measured over a 400ms window.
     * 
     * @return Momentary loudness in LUFS, or -144.0 if insufficient data
     */
    float getMomentaryLoudness() const;

    /**
     * Retrieves the Loudness Range (LRA) measurement.
     * LRA represents the variation in loudness over time.
     * 
     * @return Loudness Range in LU, or 0.0 if insufficient data
     */
    float getLoudnessRange() const;

    // Optional: Add getters for Integrated Loudness if needed later.
    // float getIntegratedLoudness() const;
    // Optional: Add getter for True Peak if needed later.
    // float getTruePeak() const;

private:
    // Core measurement state
    ebur128_state* state = nullptr; // Pointer to the libebur128 state object
    int currentNumChannels = 0;
    double currentSampleRate = 0.0;

    // Cached measurement results
    mutable float lastShortTermLUFS = -144.0f;  // Minimum valid loudness
    mutable float lastMomentaryLUFS = -144.0f;
    mutable float lastLRA = 0.0f;
    
    // Buffer for interleaved audio data
    std::vector<float> tempInterleaveBuffer; // <<< Member Variable for interleavedData

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudnessMeter)
};
