#include "OversamplingProcess.h"

namespace viator::dsp
{
    //==============================================================================
    void OversamplingProcess::prepareToPlay(
        double sampleRate,
        int samplesPerBlock,
        int numChannels,
        int oversamplingFactor)
    {
        // 0 = Off
        // 1 = 2x
        // 2 = 4x
        // 3 = 8x
        // 4 = 16x

        oversamplingFactor =
            juce::jlimit(0, 4, oversamplingFactor);

        currentFactor = oversamplingFactor;

        // Eliminar l'oversampler anterior.
        m_oversampler.reset();

        // ================================================================
        // OFF
        // ================================================================

        if (currentFactor == 0)
        {
            DBG("Oversampling: OFF");
            return;
        }

        // ================================================================
        // CREAR OVERSAMPLER IIR
        // ================================================================

        m_oversampler =
            std::make_unique<juce::dsp::Oversampling<float>>(
                static_cast<size_t>(numChannels),
                static_cast<size_t>(currentFactor),
                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                true,   // màxima qualitat
                true);  // latència entera

        // Fer que la latència total sigui un nombre enter de mostres.
        m_oversampler->setUsingIntegerLatency(true);

        // Inicialitzar.
        m_oversampler->initProcessing(
            static_cast<size_t>(samplesPerBlock));

        m_oversampler->reset();

        // ================================================================
        // DEBUG
        // ================================================================

        DBG("====================================");
        DBG("Oversampling");

        DBG("Factor real: "
            << m_oversampler->getOversamplingFactor()
            << "x");

        DBG("Fs entrada: "
            << sampleRate
            << " Hz");

        DBG("Fs interna: "
            << sampleRate *
            static_cast<double>(
                m_oversampler->getOversamplingFactor())
            << " Hz");

        DBG("Latencia OS: "
            << m_oversampler->getLatencyInSamples()
            << " mostres");

        DBG("====================================");
    }


    //==============================================================================
    juce::dsp::AudioBlock<float>
        OversamplingProcess::processSamplesUp(
            juce::AudioBuffer<float>& buffer)
    {
        juce::dsp::AudioBlock<float> block(buffer);

        if (currentFactor == 0 ||
            m_oversampler == nullptr)
        {
            return block;
        }

        const auto inputSamples =
            block.getNumSamples();

        auto oversampledBlock =
            m_oversampler->processSamplesUp(block);

        const auto ratio =
            m_oversampler->getOversamplingFactor();

        jassert(
            oversampledBlock.getNumSamples()
            ==
            inputSamples * ratio);

        return oversampledBlock;
    }


    //==============================================================================
    void OversamplingProcess::processSamplesDown(
        juce::dsp::AudioBlock<float>& block)
    {
        if (currentFactor == 0 ||
            m_oversampler == nullptr)
        {
            return;
        }

        m_oversampler->processSamplesDown(block);
    }


    //==============================================================================
    int OversamplingProcess::
        getCurrentOversamplingFactor() const
    {
        if (currentFactor == 0)
            return 1;

        return 1 << currentFactor;
    }


    //==============================================================================
    float OversamplingProcess::
        getLatencyInSamples() const
    {
        if (currentFactor == 0 ||
            m_oversampler == nullptr)
        {
            return 0.0f;
        }

        return m_oversampler->getLatencyInSamples();
    }
}

