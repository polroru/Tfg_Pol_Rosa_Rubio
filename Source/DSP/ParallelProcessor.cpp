#include "ParallelProcessor.h"

namespace viator::dsp
{
    // Prepara els buffers interns i el crossover.
    void ParallelProcessor::prepareToPlay(
        double sampleRate,
        int samplesPerBlock,
        int numChannels)
    {
        m_sampleRate = sampleRate;

        m_lowBuffer.setSize(numChannels, samplesPerBlock);
        m_highBuffer.setSize(numChannels, samplesPerBlock);

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        m_crossover.prepare(spec);

        updateCrossover();
        reset();
    }




    // Processa el bloc amb distorsió o amb distorsio + parallel.
    void ParallelProcessor::process(
        juce::dsp::AudioBlock<float>& block)
    {
        jassert(m_distortion != nullptr);

        if (m_distortion == nullptr)
            return;

        if (m_parallelEnabled)
            processParallel(block);
        else
            m_distortion->process(block);
    }



    // Reinicia el crossover i neteja els buffers.
    void ParallelProcessor::reset()
    {
        m_crossover.reset();
        m_lowBuffer.clear();
        m_highBuffer.clear();
    }



    // Assigna distorsio
    void ParallelProcessor::setDistortion(
        DistortionProcess* distortionProcess) noexcept
    {
        m_distortion = distortionProcess;
    }




    // Estat process paral·lel
    void ParallelProcessor::setParallelOnOff(
        bool enabled) noexcept
    {
        m_parallelEnabled = enabled;
    }




    // Canvia la freq tatll
    void ParallelProcessor::setCutoffFrequency(
        float cutoffFrequency) noexcept
    {
        m_cutoffFrequency = cutoffFrequency;
        updateCrossover();
    }





    // Aplica la freq tall
    void ParallelProcessor::updateCrossover()
    {
        const float maxCutoff = static_cast<float>(m_sampleRate * 0.45);

        auto cutoff = juce::jlimit(
            20.0f,
            maxCutoff,
            m_cutoffFrequency);

        m_crossover.setCutoffFrequency(cutoff);
    }



    // Separa el senyal en dos, distorsiona la baixa i suma el resultat.
    void ParallelProcessor::processParallel(
        juce::dsp::AudioBlock<float>& block)
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        // Comprova que el bloc rebut cap dins dels buffers interns.
        jassert(numChannels <= static_cast<size_t>(m_lowBuffer.getNumChannels()));
        jassert(numSamples <= static_cast<size_t>(m_lowBuffer.getNumSamples()));

        if (numChannels > static_cast<size_t>(m_lowBuffer.getNumChannels()) ||
            numSamples > static_cast<size_t>(m_lowBuffer.getNumSamples()))
        {
            return;
        }




        // Divideix el senyal en una branca baixa i una branca alta.
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            const float* input = block.getChannelPointer(channel);
            float* low = m_lowBuffer.getWritePointer(static_cast<int>(channel));
            float* high = m_highBuffer.getWritePointer(static_cast<int>(channel));

            for (size_t sample = 0; sample < numSamples; ++sample)
            {
                m_crossover.processSample(
                    static_cast<int>(channel),
                    input[sample],
                    low[sample],
                    high[sample]);
            }
        }

        juce::dsp::AudioBlock<float> lowBlock(m_lowBuffer);
        lowBlock = lowBlock.getSubsetChannelBlock(0, numChannels)
            .getSubBlock(0, numSamples);


        // Aplica distorsió a la branca baixa
        m_distortion->process(lowBlock);




        // Suma dist + branca alta
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            // Obtenim punters als canals per llegir i escriure les mostres directament.
            float* output = block.getChannelPointer(channel);
            const float* low = m_lowBuffer.getReadPointer(static_cast<int>(channel));
            const float* high = m_highBuffer.getReadPointer(static_cast<int>(channel));

            for (size_t sample = 0; sample < numSamples; ++sample)
                output[sample] = low[sample] + high[sample];
        }

        m_crossover.snapToZero();
    }
}