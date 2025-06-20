#include "DynamicsDoctor/LoudnessMeter.h"
#include <juce_core/system/juce_PlatformDefs.h> // For jassert, etc.
// #include <juce_core/logging/juce_Logger.h>  // For DBG
#include <limits> // For std::numeric_limits
#include <vector> // Added for std::vector

//==============================================================================
LoudnessMeter::LoudnessMeter()
{
    // State initialization deferred to prepare() to ensure proper sample rate and channel configuration
}

LoudnessMeter::~LoudnessMeter()
{
    if (state != nullptr)
    {
        ebur128_destroy(&state); // Clean up the libebur128 state
        state = nullptr;
    }
}

//==============================================================================
void LoudnessMeter::prepare(double sampleRate, int numChannels, int /*maxSamplesPerBlock*/)
{
    // Clean up existing state if present
    if (state != nullptr)
    {
        ebur128_destroy(&state);
        state = nullptr;
    }

    currentSampleRate = sampleRate;
    currentNumChannels = numChannels;

    // Configure libebur128 with required measurement modes:
    // - EBUR128_MODE_S: Short-term loudness (3-second window)
    // - EBUR128_MODE_M: Momentary loudness (400ms window)
    // - EBUR128_MODE_LRA: Loudness Range measurement
    unsigned int mode = EBUR128_MODE_S | EBUR128_MODE_M | EBUR128_MODE_LRA;

    state = ebur128_init(static_cast<unsigned>(numChannels),
                        static_cast<unsigned>(sampleRate),
                        mode);

    if (state == nullptr)
    {
        jassertfalse; // Failed to initialize libebur128
        return;
    }

    // Configure channel mapping for standard layouts
    if (numChannels == 1 && state != nullptr) {
        ebur128_set_channel(state, 0, EBUR128_CENTER);
    } else if (numChannels == 2 && state != nullptr) {
        ebur128_set_channel(state, 0, EBUR128_LEFT);
        ebur128_set_channel(state, 1, EBUR128_RIGHT);
    }

    reset();
}

//==============================================================================
void LoudnessMeter::reset()
{
    // Reset cached measurement values to their initial states
    lastShortTermLUFS = -144.0f;  // Minimum valid loudness
    lastMomentaryLUFS = -144.0f;
    lastLRA = 0.0f;
    // Note: A 'harder' reset for libebur128's internal LRA history would be to call prepare() again.
    // For now, this resets our cached values. libebur128's LRA is continuous.
}

//==============================================================================
void LoudnessMeter::processBlock(const juce::AudioBuffer<float>& buffer)
{
    if (state == nullptr || buffer.getNumSamples() == 0)
    {
        return; // Not prepared or empty buffer
    }
    
    // Validate channel configuration
    jassert(buffer.getNumChannels() == currentNumChannels);
    if (buffer.getNumChannels() != currentNumChannels)
    {
        DBG("LoudnessMeter::processBlock channel count mismatch! Expected: " << currentNumChannels << " Got: " << buffer.getNumChannels());
        return; // Mismatch, can't process
    }
        
    const int numFrames = buffer.getNumSamples(); // In libebur128, 'frames means samples per channel.
    const int numChans = currentNumChannels;
    
    // Prepare interleaved buffer for libebur128
    std::vector<float> interleavedData;
    interleavedData.resize(static_cast<size_t>(numFrames * numChans));
    
    // Convert planar audio data to interleaved format
    for (int i = 0; i < numFrames; ++i)
    {
        for (int ch = 0; ch < numChans; ++ch)
        {
            const float* channelReadPtr = buffer.getReadPointer(ch);
            float sampleValue = channelReadPtr[i];
            interleavedData[static_cast<size_t>(i * numChans + ch)] = sampleValue;
        }
    }
    
    // Process the audio block with libebur128
    int err = ebur128_add_frames_float(state, interleavedData.data(), static_cast<size_t>(numFrames));
    
    if (err != 0)
    {
        DBG("libebur128_add_frames_float error code: " << err);
        // In a release build, you might want to log this or handle it more gracefully than just asserting. For debugging, an assert is fine.
        jassertfalse;  // Error in libeur128 processing
    }
    
    // Note: The getter methods will be called from PluginProcessor to retrieve values.
    // We don't necessarily need to update lastShortTermLUFS etc. here, as the getters
    // will fetch the latest from libebur128.
}

//==============================================================================
float LoudnessMeter::getShortTermLoudness() const
{
    if (state == nullptr) return -std::numeric_limits<float>::infinity();
    
    double lufs_s;
    if (ebur128_loudness_shortterm(state, &lufs_s) == 0)
    {
        lastShortTermLUFS = static_cast<float>(lufs_s);
        // Return -infinity for values below -140 LUFS (effectively silence)
        return (lufs_s < -140.0) ? -std::numeric_limits<float>::infinity() : static_cast<float>(lufs_s);
    }
    // DBG("Error getting short-term loudness from libebur128");
    return lastShortTermLUFS; // Return cached value on error
}

//==============================================================================
float LoudnessMeter::getMomentaryLoudness() const
{
    if (state == nullptr) return -std::numeric_limits<float>::infinity();
    
    double lufs_m;
    if (ebur128_loudness_momentary(state, &lufs_m) == 0)
    {
        lastMomentaryLUFS = static_cast<float>(lufs_m);
        // Return -infinity for values below -140 LUFS (effectively silence)
        return (lufs_m < -140.0) ? -std::numeric_limits<float>::infinity() : static_cast<float>(lufs_m);
    }
    // DBG("Error getting momentary loudness from libebur128");
    return lastMomentaryLUFS; // Return cached value on error
}

//==============================================================================
float LoudnessMeter::getLoudnessRange() const
{
    if (state == nullptr) return 0.0f;  // LRA is typically positive or zero
    
    double lra_val;
    if (ebur128_loudness_range(state, &lra_val) == 0)
    {
        lastLRA = static_cast<float>(lra_val);
        return static_cast<float>(lra_val);
    }
    // DBG ("Error getting loudness range from libebur128");
    return lastLRA; // Return cached value on error
}
