#pragma once

#include <juce_dsp/juce_dsp.h>

namespace viator::dsp
{
    class DistortionProcess
    {
    public:
        void process(juce::dsp::AudioBlock<float>& block);

        void setGain(float newGain);
        void setThreshold(float newThreshold);
        void setDistortionType(int newDistortionType);
        void setADAAType(int newType);

        void reset();

    private:
        static constexpr double adaaEpsilon = 1.0e-10;

        float gain = 1.0f;
        float threshold = 0.5f;

        int distortionType = 0;
        int adaaType = 0;

        // Mostres utilitzades anteriorment
        double previousInput = 0.0;
        double previousInput2 = 0.0;


        using DistortionFunction =
            float (DistortionProcess::*)(float);

        using AntiderivativeFunction =
            double (DistortionProcess::*)(double);

        //======================================================================
        // Hard clipping
        //======================================================================

        float hardClip(float x);

        double hardClipAntiderivada(double x);
        double hardClipAntiderivada2(double x);

        //======================================================================
        // Tanh
        //======================================================================

        float tanhClip(float x);

        double tanhClipAntiderivada(double x);

        //======================================================================
        // Cubic clipping
        //======================================================================

        float cubicClip(float x);

        double cubicClipAntiderivada(double x);
        double cubicClipAntiderivada2(double x);

        //======================================================================
        // Arctangent clipping
        //======================================================================

        float arctanClip(float x);

        double arctanClipAntiderivada(double x);
        double arctanClipAntiderivada2(double x);

        //======================================================================
        // Asymmetric clipping
        //======================================================================

        float asymmetricClip(float x);

        double asymmetricClipAntiderivada(double x);
        double asymmetricClipAntiderivada2(double x);

        //======================================================================
        // ADAA
        //======================================================================

        float processADAA1(
            float x,
            DistortionFunction function,
            AntiderivativeFunction antiderivative);

        float processADAA2(
            float x,
            DistortionFunction function,
            AntiderivativeFunction antiderivative,
            AntiderivativeFunction secondAntiderivative);
    };
}