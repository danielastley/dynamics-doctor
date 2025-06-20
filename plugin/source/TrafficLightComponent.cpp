#include "DynamicsDoctor/TrafficLightComponent.h"
#include "DynamicsDoctor/Constants.h" // Needed for DynamicsStatus, helper functions, Palette

//==============================================================================
TrafficLightComponent::TrafficLightComponent()
{
    // Component is purely visual with no child components or resources to initialize
}

TrafficLightComponent::~TrafficLightComponent()
{
    // No resources to clean up
}

//==============================================================================
void TrafficLightComponent::paint (juce::Graphics& g)
{
    // Calculate component dimensions and layout
    auto bounds = getLocalBounds().toFloat();

    // Configure light spacing and sizing
    constexpr float spacing = 5.0f;  // Vertical and horizontal spacing between lights
    const float totalHeightForLights = bounds.getHeight() - (spacing * 4.0f);  // Account for gaps
    const float lightDiameter = juce::jmin(totalHeightForLights / 3.0f,  // Equal height per light
                                         bounds.getWidth() - (spacing * 2.0f));  // Respect width

    // Validate available space
    if (lightDiameter <= 0.0f)
        return;

    // Calculate light positions
    const float x = bounds.getCentreX() - (lightDiameter / 2.0f);
    float currentY = spacing;

    // Define bounds for each light (top to bottom)
    juce::Rectangle<float> topLightBounds(x, currentY, lightDiameter, lightDiameter);
    currentY += lightDiameter + spacing;
    juce::Rectangle<float> midLightBounds(x, currentY, lightDiameter, lightDiameter);
    currentY += lightDiameter + spacing;
    juce::Rectangle<float> botLightBounds(x, currentY, lightDiameter, lightDiameter);

    // Draw lights in standard traffic light order:
    // Red (Loss) at top, Yellow (Reduced) in middle, Green (Ok) at bottom
    paintLight(g, topLightBounds, DynamicsStatus::Loss);
    paintLight(g, midLightBounds, DynamicsStatus::Reduced);
    paintLight(g, botLightBounds, DynamicsStatus::Ok);
}

//==============================================================================
void TrafficLightComponent::paintLight (juce::Graphics& g, const juce::Rectangle<float>& bounds, DynamicsStatus lightTargetStatus)
{
    // Get visual properties based on light's target status and current component state
    const juce::Colour fillColour = TrafficLightMetrics::getLightColour(lightTargetStatus, currentStatus);
    const juce::Colour borderColour = TrafficLightMetrics::getLightBorderColour(lightTargetStatus, currentStatus);
    const float borderThickness = TrafficLightMetrics::LightBorderThickness;

    // Draw light with fill and border
    g.setColour(fillColour);
    g.fillEllipse(bounds);

    g.setColour(borderColour);
    g.drawEllipse(bounds, borderThickness);
}

//==============================================================================
void TrafficLightComponent::resized()
{
    // No child components to resize
    // Component automatically adapts to size changes in paint()
}

//==============================================================================
void TrafficLightComponent::setStatus (DynamicsStatus newStatus)
{
    // Update status and trigger repaint only if changed
    if (currentStatus != newStatus)
    {
        currentStatus = newStatus;
        repaint();
    }
}
