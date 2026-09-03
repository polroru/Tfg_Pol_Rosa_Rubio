#pragma once

#include <JuceHeader.h>

namespace viator::parameters
{
    //==========================================================================
	// IDs de parametres i noms de paràmetres

    inline constexpr auto oversamplingID = "OVERSAMPLING";
    inline constexpr auto oversamplingName = "Oversampling";

    inline constexpr auto gainID = "GAIN";
    inline constexpr auto gainName = "gain";

    inline constexpr auto levelID = "LEVEL";
    inline constexpr auto levelName = "level";

    inline constexpr auto distortionTypeID = "DISTORTIONTYPE";
    inline constexpr auto distortionTypeName = "Distortion Type";

    inline constexpr auto thresholdID = "THRESHOLD";
    inline constexpr auto thresholdName = "threshold";

    inline constexpr auto parallelID = "PARALLEL";
    inline constexpr auto parallelName = "parallel";

    inline constexpr auto adaaID = "ADAA";
    inline constexpr auto adaaName = "adaa";



    //==========================================================================
	//Funcio per a crear el layout de paràmetres1
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==========================================================================
	// Estrcutura Parameters, que conté un punter a un float atòmic per a l'oversampling
	//punter atòmc es utilitzat per a accedir al valor del paràmetre d'oversampling de manera segura en entorns multithreading

    struct Parameters
    {
		// constructor de la estructura Parameters, que rep com a paràmetre un objecte AudioProcessorValueTreeState
        explicit Parameters(juce::AudioProcessorValueTreeState& apvts);

        std::atomic<float>* oversampling = nullptr;
        std::atomic<float>* gain = nullptr;
        std::atomic<float>* level = nullptr;
        std::atomic<float>* distortionType = nullptr;
        std::atomic<float>* threshold = nullptr;
        std::atomic<float>* parallel = nullptr;
        std::atomic<float>* adaa = nullptr;

    };
}