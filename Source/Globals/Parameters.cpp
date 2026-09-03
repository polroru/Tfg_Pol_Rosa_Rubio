#include "Parameters.h"


//tipus de retorn: juce::AudioProcessorValueTreeState::ParameterLayout 
//tipus de funció: viator::parameters::createParameterLayout
juce::AudioProcessorValueTreeState::ParameterLayout viator::parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        oversamplingID,
        oversamplingName,
        juce::StringArray{ "Off", "2x", "4x", "8x", "16x" },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        gainID,
        gainName,
        juce::NormalisableRange<float>(1.0f, 10.0f, 0.01f),
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        levelID,
        levelName,
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        distortionTypeID,
        distortionTypeName,
        juce::StringArray{
            "Off",
            "Hard Clip",
            "Tanh",
            "Cubic",
            "Arctan",
            "Asymmetric"
        },
        0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        thresholdID,
        thresholdName,
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        parallelID,
        parallelName,
        false));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        adaaID,
        "ADAA",
        juce::StringArray{ "Off", "ADAA1", "ADAA2" },
        0));

    return layout;
}



//==============================================================================
// Constructor de Parameters

viator::parameters::Parameters::Parameters(
    juce::AudioProcessorValueTreeState& apvts)
{
    oversampling = apvts.getRawParameterValue(oversamplingID);
    gain = apvts.getRawParameterValue(gainID);
	level = apvts.getRawParameterValue(levelID);
    distortionType = apvts.getRawParameterValue(distortionTypeID);
    threshold = apvts.getRawParameterValue(thresholdID);
    parallel = apvts.getRawParameterValue(parallelID);
    adaa = apvts.getRawParameterValue(adaaID);
}