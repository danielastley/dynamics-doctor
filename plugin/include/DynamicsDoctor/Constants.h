#pragma once

// Core JUCE modules
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <string>

//==============================================================================
/**
 * UI Color definitions for the plugin.
 * All colors are defined with full opacity (FF prefix) unless specified otherwise.
 */
namespace Palette
{
    // Main UI colors
    const juce::Colour Primary        = juce::Colour::fromString ("FF333333"); // Main background
    const juce::Colour Secondary      = juce::Colour::fromString ("FFDDDDDD"); // Main text/foreground
    const juce::Colour Ok             = juce::Colour::fromString ("FF008080"); // Success state (teal)
    const juce::Colour Reduced        = juce::Colour::fromString ("FFFFA500"); // Warning state (orange)
    const juce::Colour Loss           = juce::Colour::fromString ("FFFF0000"); // Error state (red)
    const juce::Colour Muted          = juce::Colours::grey;                   // Inactive state
    const juce::Colour DisabledText   = Muted.withAlpha(0.6f);                // Disabled text

    // Legacy color aliases
    const juce::Colour Background     = Primary;
    const juce::Colour Foreground     = Secondary;
}

//==============================================================================
/**
 * Plugin state definitions.
 * Represents the different dynamic range states that can be detected.
 */
enum class DynamicsStatus
{
    Ok,       // Normal dynamic range
    Reduced,  // Reduced dynamic range
    Loss,     // Significant dynamic range loss
    Bypassed, // Plugin is bypassed
    Measuring, // Currently measuring LRA
    AwaitingAudio // Waiting for audio signal
};

//==============================================================================
/**
 * Preset configuration structure.
 * Defines the dynamic range thresholds and target ranges for different music genres.
 */
struct DynamicsPreset
{
    std::string id;         // Unique identifier for parameter system
    juce::String label;     // Display name in UI

    // LRA thresholds in LU (Loudness Units)
    float lraThresholdRed;    // Red light threshold
    float lraThresholdAmber;  // Amber light threshold
    float targetLraMin;       // Minimum target LRA
    float targetLraMax;       // Maximum target LRA
};

//==============================================================================
/**
 * Predefined dynamic range presets for different music genres.
 * Each preset defines specific LRA thresholds and target ranges.
 */
const std::vector<DynamicsPreset> presets = {
    // id,         label,            red,    amber,  min,    max
    { "edm",       "EDM/Club",       3.0f,   3.6f,   3.0f,   8.0f },
    { "pop_rock",  "Pop/Rock",       4.0f,   4.8f,   4.0f,   9.0f },
    { "classical", "Classical",      6.0f,   7.2f,   6.0f,  22.0f }
};

//==============================================================================
/**
 * Parameter identifiers for the plugin's audio processor.
 * Used to identify parameters in the AudioProcessorValueTreeState.
 */
namespace ParameterIDs
{
    const juce::ParameterID preset   { "preset", 1 };
    const juce::ParameterID peak     { "peak",   1 };
    const juce::ParameterID lra      { "lra",    1 };
    const juce::ParameterID resetLra { "resetLra", 1 };
}

//==============================================================================
/**
 * Default values for plugin parameters.
 * Used when initializing the plugin or resetting parameters.
 */
namespace ParameterDefaults
{
    const int  preset = 1;                    // Default to Pop/Rock preset
    const float peak = -100.0f;               // Initial peak level
    const float lra = 0.0f;                   // Initial LRA value
    constexpr float LRA_MEASURING_DURATION = 6.0f; // LRA measurement period
}

//==============================================================================
/**
 * Status-related utility functions.
 * Provides consistent colors and messages for different plugin states.
 */
inline juce::Colour getStatusColour(DynamicsStatus status)
{
    switch (status)
    {
        case DynamicsStatus::Ok:        return Palette::Ok;
        case DynamicsStatus::Reduced:   return Palette::Reduced;
        case DynamicsStatus::Loss:      return Palette::Loss;
        case DynamicsStatus::Measuring: return Palette::Secondary.withAlpha(0.6f);
        case DynamicsStatus::AwaitingAudio: return Palette::Ok; // Green for awaiting audio
        case DynamicsStatus::Bypassed:  /* fallthrough */
        default:                        return Palette::Muted;
    }
}

inline juce::String getStatusMessage(DynamicsStatus status)
{
    switch (status)
    {
        case DynamicsStatus::Ok:        return "Dynamics: OK";
        case DynamicsStatus::Reduced:   return "Dynamics: Reduced";
        case DynamicsStatus::Loss:      return "Dynamics: Loss Risk";
        case DynamicsStatus::Measuring: return "Measuring LRA...";
        case DynamicsStatus::AwaitingAudio: return "Awaiting Audio...";
        case DynamicsStatus::Bypassed:  /* fallthrough */
        default:                        return "Monitoring Bypassed";
    }
}

//==============================================================================
/**
 * Traffic light display configuration and utilities.
 * Defines visual properties and behavior of the traffic light component.
 */
namespace TrafficLightMetrics
{
    // Visual properties
    constexpr float LightBorderThickness = 2.0f;
    constexpr float BypassedAlpha = 0.3f;
    constexpr float InactiveAlpha = 0.3f;
    constexpr float ActiveBorderDarkenFactor = 0.5f;
    constexpr float InactiveBackgroundBrightnessFactor = 0.2f;
    constexpr float BypassedBorderDarkenFactor = 0.3f;

    /**
     * Determines the color of a traffic light based on current plugin state.
     * @param lightTargetStatus The status this light represents
     * @param currentActualStatus The current plugin status
     * @return The appropriate color for the light
     */
    inline juce::Colour getLightColour(DynamicsStatus lightTargetStatus, DynamicsStatus currentActualStatus)
    {
        if (currentActualStatus == DynamicsStatus::Bypassed)
            return Palette::Muted.withAlpha(BypassedAlpha);
        
        if (currentActualStatus == DynamicsStatus::Measuring)
            return Palette::Background.brighter(InactiveBackgroundBrightnessFactor);

        if (currentActualStatus == lightTargetStatus)
            return getStatusColour(lightTargetStatus);

        return Palette::Background.brighter(InactiveBackgroundBrightnessFactor);
    }

    /**
     * Determines the border color of a traffic light based on current plugin state.
     * @param lightTargetStatus The status this light represents
     * @param currentActualStatus The current plugin status
     * @return The appropriate border color for the light
     */
    inline juce::Colour getLightBorderColour(DynamicsStatus lightTargetStatus, DynamicsStatus currentActualStatus)
    {
        if (currentActualStatus == DynamicsStatus::Bypassed)
            return Palette::Muted.darker(BypassedBorderDarkenFactor);

        if (currentActualStatus == DynamicsStatus::Measuring)
            return Palette::Foreground.withAlpha(InactiveAlpha);

        if (currentActualStatus == lightTargetStatus)
            return getStatusColour(lightTargetStatus).darker(ActiveBorderDarkenFactor);

        return Palette::Foreground.withAlpha(InactiveAlpha);
    }
}
