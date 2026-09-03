#pragma once

#include <JuceHeader.h>
#include "DistortionProcess.h"

namespace viator::dsp
{
    class ParallelProcessor
    {
    public:
        ParallelProcessor() = default;

        void prepareToPlay(
            double sampleRate,
            int samplesPerBlock,
            int numChannels);

        void reset();

        void process(juce::dsp::AudioBlock<float>& block);

        void setDistortion(DistortionProcess* distortionProcess) noexcept;

        void setParallelOnOff(bool enabled) noexcept;

        void setCutoffFrequency(float cutoffFrequency) noexcept;






    private:
        void processParallel(juce::dsp::AudioBlock<float>& block);

        void updateCrossover();

        // Distorsió del plugin processor
        DistortionProcess* m_distortion = nullptr;

        bool m_parallelEnabled = false;

        double m_sampleRate = 44100.0;
        float m_cutoffFrequency = 6000.0f;

        // Buffers interns de les dues branques.
        juce::AudioBuffer<float> m_lowBuffer;
        juce::AudioBuffer<float> m_highBuffer;

        /*
            Filtre de separació bandes.

            Genera simultàniament:
            - sortida pas baix;
            - sortida pas alt.
        */


        juce::dsp::LinkwitzRileyFilter<float> m_crossover;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            ParallelProcessor)
    };


}