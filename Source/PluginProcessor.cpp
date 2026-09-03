/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RosaRosaAudioProcessor::RosaRosaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::mono(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::mono(), true)
#endif
    ),
    m_tree_state(*this, nullptr, "PARAMETERS",
        viator::parameters::createParameterLayout()),
    m_parameters(m_tree_state)
#endif
{
    DBG("PLUGIN CARREGAT");
}

RosaRosaAudioProcessor::~RosaRosaAudioProcessor()
{
}

//==============================================================================
const juce::String RosaRosaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RosaRosaAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RosaRosaAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RosaRosaAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RosaRosaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RosaRosaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int RosaRosaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RosaRosaAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RosaRosaAudioProcessor::getProgramName(int index)
{
    return {};
}

void RosaRosaAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}






//==============================================================================PREPARACIÓ DE PROCESSBLOCK

// Prepara el processador : guardo fs, la mida del bloc i es configuro cadena de processat amb el factor L d'oversampling
void RosaRosaAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    m_samplesPerBlock = samplesPerBlock;
    m_numChannels = juce::jmax(1, getTotalNumOutputChannels()); //forcem Mono

    // El ParallelProcessor utilitza el mateix mòdul de distorsió.
    m_parallelProcessor.setDistortion(&m_distortionProcess);

    const int oversamplingIndex = static_cast<int>(m_parameters.oversampling->load());

    prepareProcess(oversamplingIndex);
    DBG(juce::String("Fs rebuda del DAW: ") + juce::String(sampleRate, 0));
    m_distortionProcess.reset(); //Reinicia estat modul distorsió
}






void RosaRosaAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RosaRosaAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect

    juce::ignoreUnused(layouts);
    return true;

#else

    // El plugin només treballa amb un canal mono.
    if (layouts.getMainOutputChannelSet()
        != juce::AudioChannelSet::mono())
    {
        return false;
    }

#if !JucePlugin_IsSynth

    // L'entrada també ha de ser mono.
    if (layouts.getMainInputChannelSet()
        != juce::AudioChannelSet::mono())
    {
        return false;
    }

#endif

    return true;

#endif
}
#endif






//==============================================================================PROCESSBLOCK
// Llegeix els paràmetres, aplica oversampling i processa la distorsió (dins paralel processor)
void RosaRosaAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);






    // Comprovació temporal del nombre de canals
    static int previousChannels = -1;

    if (previousChannels != buffer.getNumChannels())
    {
        DBG("NUMERO DE CANALS = "+ juce::String(buffer.getNumChannels()));

        previousChannels = buffer.getNumChannels();
    }


    // Netejar canals de sortida sense entrada corresponent
    for (int channel = getTotalNumInputChannels();
        channel < getTotalNumOutputChannels();
        ++channel)
    {
        buffer.clear(
            channel,
            0,
            buffer.getNumSamples());
    }







    // Llegir els paràmetres de processament
    const float gain = m_parameters.gain->load();

    const float level = m_parameters.level->load();

    const float threshold = m_parameters.threshold->load();

    const int distortionType = static_cast<int>(m_parameters.distortionType->load());

    const bool parallelEnabled = m_parameters.parallel->load() >= 0.5f;

    const int oversamplingIndex = static_cast<int>(m_parameters.oversampling->load());

    const int adaa = static_cast<int>(m_parameters.adaa->load());


    if (oversamplingIndex != m_currentOversampling)
    {
        DBG("Canvi d'oversampling -> Fs host: "
            << m_sampleRate
            << " Hz | Factor OS: "
            << (1 << oversamplingIndex)
            << "x");

        prepareProcess(oversamplingIndex);
    }


    // ================================================================
    // PREPARAR BLOC I APLICAR OVERSAMPLING
    // ================================================================

    juce::dsp::AudioBlock<float> originalBlock(buffer);

    auto processingBlock =
        m_oversampling.processSamplesUp(buffer);


    // ================================================================
    // DISTORSIO
    // ================================================================

    if (distortionType != 0)
    {
        m_distortionProcess.setDistortionType(distortionType);

        m_distortionProcess.setGain(gain);

        m_distortionProcess.setThreshold(threshold);

        m_distortionProcess.setADAAType(adaa);

        m_parallelProcessor.setParallelOnOff(parallelEnabled);

        m_parallelProcessor.process(processingBlock);
    }


    // ================================================================
    // DOWNSAMPLING
    // ================================================================

    m_oversampling.processSamplesDown(originalBlock);


    // ================================================================
    // NIVELL
    // ================================================================

    if (distortionType != 0)
    {
        buffer.applyGain(level);
    }
}



//==============================================================================
bool RosaRosaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RosaRosaAudioProcessor::createEditor()
{
    return new RosaRosaAudioProcessorEditor(*this);
}

//==============================================================================
void RosaRosaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RosaRosaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RosaRosaAudioProcessor();
}


juce::AudioProcessorValueTreeState& RosaRosaAudioProcessor::getTreeState()
{
    return m_tree_state;
}










//============================================================================== FUNCIONS CUSTOM
void RosaRosaAudioProcessor::prepareProcess(
    int oversamplingIndex)
{
    m_currentOversampling = oversamplingIndex;

    // 0 -> 1x
    // 1 -> 2x
    // 2 -> 4x
    // 3 -> 8x
    // 4 -> 16x
    const int oversamplingRatio =
        1 << oversamplingIndex;

    // ================================================================
    // OVERSAMPLING
    // ================================================================

    m_oversampling.prepareToPlay(
        m_sampleRate,
        m_samplesPerBlock,
        m_numChannels,
        oversamplingIndex);

    // ================================================================
    // COMUNICAR LATENCIA AL HOST
    // ================================================================

    const int latencySamples =
        juce::roundToInt(
            m_oversampling.getLatencyInSamples());

    setLatencySamples(latencySamples);

    DBG("Latencia comunicada al host: "
        << latencySamples
        << " mostres");


    // ================================================================
    // PARALLEL PROCESSOR
    // ================================================================

    m_parallelProcessor.prepareToPlay(
        m_sampleRate *
        static_cast<double>(oversamplingRatio),

        m_samplesPerBlock *
        oversamplingRatio,

        m_numChannels);
}