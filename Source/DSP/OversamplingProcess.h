#pragma once

#include <juce_dsp/juce_dsp.h>
#include <memory>

namespace viator::dsp
{
    class OversamplingProcess
    {
    public:

        void prepareToPlay(
            double sampleRate,
            int samplesPerBlock,
            int numChannels,
            int oversamplingFactor);

        juce::dsp::AudioBlock<float> processSamplesUp(
            juce::AudioBuffer<float>& buffer);

        void processSamplesDown(
            juce::dsp::AudioBlock<float>& block);

        int getCurrentOversamplingFactor() const;

        float getLatencyInSamples() const;

    private:

        std::unique_ptr<juce::dsp::Oversampling<float>>
            m_oversampler;

        int currentFactor = 0;
    };
}