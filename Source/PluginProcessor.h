/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/OversamplingProcess.h"
#include "Globals/Parameters.h"
#include "DSP/DistortionProcess.h"
#include "DSP/ParallelProcessor.h"



//==============================================================================
/**
*/
class RosaRosaAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    RosaRosaAudioProcessor();
    ~RosaRosaAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;














    juce::AudioProcessorValueTreeState& getTreeState();

private:
    //==============================================================================
	


    //==============================================================================\\
    //================================PARAMETERS====================================\\







    // Objecte de la classe AudioProcessorValueTreeState per a gestionar els paràmetres del plugin
    juce::AudioProcessorValueTreeState m_tree_state;

	// Objecte que facilita l'accés als paràmetres del plugin
	viator::parameters::Parameters m_parameters;









    //==============================================================================\\
    //====================================DSP========================================\\



    void prepareProcess(int oversamplingIndex);



    double m_sampleRate = 44100.0f;
    int m_samplesPerBlock = 0;
    int m_numChannels = 2;
    int m_currentOversampling = -1;


    // Objecte de la classe OversamplingProcess per a gestionar l'oversampling
    viator::dsp::OversamplingProcess m_oversampling;
    // Objecte de la classe DistortionProcess per a gestionar la distorsió
    viator::dsp::DistortionProcess m_distortionProcess;
    // Objecte de la classe ParallelProcessor per a gestionar el processament paral·lel
    viator::dsp::ParallelProcessor m_parallelProcessor;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RosaRosaAudioProcessor)
};
