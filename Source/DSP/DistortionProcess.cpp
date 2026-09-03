#include "DistortionProcess.h"
#include <cmath>
#include <cstdint>

namespace viator::dsp
{
    void DistortionProcess::process(juce::dsp::AudioBlock<float>& block)
    {
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            auto* channelData = block.getChannelPointer(channel);

            for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
            {
                float x = channelData[sample];

                // Apliquem el gain abans de la funció no lineal.
                if (distortionType != 0)
                    x *= gain;

                switch (distortionType)
                {
                    // OFF
                case 0:
                    break;
                    // HARD CLIP
                case 1:
                {
                    if (adaaType == 0)
                    {
                        x = hardClip(x);
                    }
                    else if (adaaType == 1)
                    {
                        x = processADAA1(
                            x,
                            &DistortionProcess::hardClip,
                            &DistortionProcess::hardClipAntiderivada);
                    }
                    else if (adaaType == 2)
                    {
                        x = processADAA2(
                            x,
                            &DistortionProcess::hardClip,
                            &DistortionProcess::hardClipAntiderivada,
                            &DistortionProcess::hardClipAntiderivada2);
                    }

                    break;
                }
                // TANH
                case 2:
                {
                    if (adaaType == 0)
                    {
                        x = tanhClip(x);
                    }
                    else if (adaaType == 1)
                    {
                        // Tanh només disposa d'ADAA1.
                        x = processADAA1(
                            x,
                            &DistortionProcess::tanhClip,
                            &DistortionProcess::tanhClipAntiderivada);
                    }
                    else
                    {
                        // Aquesta situació no hauria de produir-se perquè
                        // setADAAType() impedeix seleccionar ADAA2 per a tanh.
                        jassertfalse;
                        x = tanhClip(x);
                    }

                    break;
                }
                // CUBIC CLIP
                case 3:
                {
                    if (adaaType == 0)
                    {
                        x = cubicClip(x);
                    }
                    else if (adaaType == 1)
                    {
                        x = processADAA1(
                            x,
                            &DistortionProcess::cubicClip,
                            &DistortionProcess::cubicClipAntiderivada);
                    }
                    else if (adaaType == 2)
                    {
                        x = processADAA2(
                            x,
                            &DistortionProcess::cubicClip,
                            &DistortionProcess::cubicClipAntiderivada,
                            &DistortionProcess::cubicClipAntiderivada2);
                    }

                    break;
                }
                // ARCTAN
                case 4:
                {
                    if (adaaType == 0)
                    {
                        x = arctanClip(x);
                    }
                    else if (adaaType == 1)
                    {
                        x = processADAA1(
                            x,
                            &DistortionProcess::arctanClip,
                            &DistortionProcess::arctanClipAntiderivada);
                    }
                    else if (adaaType == 2)
                    {
                        x = processADAA2(
                            x,
                            &DistortionProcess::arctanClip,
                            &DistortionProcess::arctanClipAntiderivada,
                            &DistortionProcess::arctanClipAntiderivada2);
                    }

                    break;
                }
                // ASYMMETRIC CLIP
                case 5:
                {
                    if (adaaType == 0)
                    {
                        x = asymmetricClip(x);
                    }
                    else if (adaaType == 1)
                    {
                        x = processADAA1(
                            x,
                            &DistortionProcess::asymmetricClip,
                            &DistortionProcess::asymmetricClipAntiderivada);
                    }
                    else if (adaaType == 2)
                    {
                        x = processADAA2(
                            x,
                            &DistortionProcess::asymmetricClip,
                            &DistortionProcess::asymmetricClipAntiderivada,
                            &DistortionProcess::asymmetricClipAntiderivada2);
                    }

                    break;
                }

                default:
                    break;
                }

                channelData[sample] = x;
            }
        }
    }

    // Actualitza el guany aplicat abans de la funció no lineal.
    void DistortionProcess::setGain(float newGain)
    {
        gain = newGain;
    }

    // Actualitza el llindar utilitzat pels algorismes de clipping.
    void DistortionProcess::setThreshold(float newThreshold)
    {
        threshold = newThreshold;
    }

    // Selecciona el tipus de funció no lineal.
    void DistortionProcess::setDistortionType(int newDistortionType)
    {
        if (distortionType == newDistortionType)
            return;

        distortionType = newDistortionType;

        // La tangent hiperbòlica només disposa d'ADAA1.
        // Si s'hi arriba tenint ADAA2 seleccionat, es canvia explícitament
        // a ADAA1 per evitar que la interfície indiqui un mètode inexistent.
        if (distortionType == 2 && adaaType == 2)
        {
            adaaType = 1;
            DBG("Tanh no disposa d'ADAA2: s'utilitza ADAA1.");
        }

        // Les mostres anteriors pertanyen a una altra funció no lineal.
        reset();
    }

    // Selecciona el tipus de adaa.
    void DistortionProcess::setADAAType(int newType)
    {
        newType = juce::jlimit(0, 2, newType);

        // Tanh només té implementat ADAA1.
        if (distortionType == 2 && newType == 2)
        {
            DBG("Tanh no disposa d'ADAA2: s'utilitza ADAA1.");
            newType = 1;
        }

        if (adaaType == newType)
            return;

        adaaType = newType;

        // ADAA1 i ADAA2 utilitzen historials diferents.
        reset();
    }

    // Reinicia l'estat necessari per ADAA.
    void DistortionProcess::reset()
    {
        previousInput = 0.0;
        previousInput2 = 0.0;
    }

    // Limita simètricament la senyal entre -threshold i +threshold.
    float DistortionProcess::hardClip(float x)
    {
        if (x > threshold)
            return threshold;

        if (x < -threshold)
            return -threshold;

        return x;
    }
    // Primera antiderivada del hard clipping, utilitzada per ADAA1.

    double DistortionProcess::hardClipAntiderivada(double x)
    {
        const double thresholdSquared = threshold * threshold;

        if (x < -threshold)
        {
            return -threshold * x
                - 0.5f * thresholdSquared;
        }

        if (x > threshold)
        {
            return threshold * x
                - 0.5f * thresholdSquared;
        }

        return 0.5f * x * x;
    }
    // Segona antiderivada del hard clipping, utilitzada per ADAA2.

    double DistortionProcess::hardClipAntiderivada2(double x)
    {
        const double thresholdQuadrat = threshold * threshold;
        const double thresholdCubed = thresholdQuadrat * threshold;

        if (x < -threshold)
        {
            return -0.5f * threshold * x * x
                - 0.5f * thresholdQuadrat * x
                - thresholdCubed / 6.0f;
        }

        if (x > threshold)
        {
            return 0.5f * threshold * x * x
                - 0.5f * thresholdQuadrat * x
                + thresholdCubed / 6.0f;
        }

        return (x * x * x) / 6.0f;
    }

    // Aplica una saturació suau mitjançant la tangent hiperbòlica.
    float DistortionProcess::tanhClip(float x)
    {
        return std::tanh(x);
    }
    // Antiderivada de tanh, necessària per aplicar ADAA1.

    double DistortionProcess::tanhClipAntiderivada(double x)
    {
        // Forma estable de log(cosh(x)). Evita que cosh(x) desbordi
        // quan el guany d'entrada és elevat.
        const double valorAbsolut = std::abs(x);

        return valorAbsolut
            + std::log1p(std::exp(-2.0 * valorAbsolut))
            - std::log(2.0);
    }

    // Aplica una funció cúbica dins de l'interval [-1, 1].
    // Fora d'aquest interval, la sortida queda limitada a ±2/3.
    float DistortionProcess::cubicClip(float x)
    {
        if (x > 1.0f)
            return 2.0f / 3.0f;

        if (x < -1.0f)
            return -2.0f / 3.0f;

        return x - (x * x * x) / 3.0f;
    }

    // Primera antiderivada del cubic clipping, utilitzada per ADAA1.
    double DistortionProcess::cubicClipAntiderivada(double x)
    {
        if (x < -1.0)
        {
            return -(2.0 / 3.0) * x
                - 0.25;
        }

        if (x > 1.0)
        {
            return (2.0 / 3.0) * x
                - 0.25;
        }

        const double x2 = x * x;
        const double x4 = x2 * x2;

        return 0.5f * x2
            - x4 / 12.0f;
    }

    // Segona antiderivada del cubic clipping.
   // Necessària per aplicar ADAA de segon ordre.
    double DistortionProcess::cubicClipAntiderivada2(double x)
    {
        if (x < -1.0)
        {
            return -(x * x) / 3.0
                - x / 4.0
                - 1.0 / 15.0;
        }

        if (x > 1.0)
        {
            return (x * x) / 3.0
                - x / 4.0
                + 1.0 / 15.0;
        }

        const double x2 = x * x;
        const double x3 = x2 * x;
        const double x5 = x3 * x2;

        return x3 / 6.0f
            - x5 / 60.0f;
    }

    // Aplica una saturació suau basada en la funció arc tangent.
    float DistortionProcess::arctanClip(float x)
    {
        return (2.0f / juce::MathConstants<float>::pi)
            * std::atan(x);
    }

    // Primera antiderivada de l'arctangent, utilitzada per ADAA1.
    double DistortionProcess::arctanClipAntiderivada(double x)
    {
        const double factor =
            2.0 / juce::MathConstants<double>::pi;

        return factor *
            (
                x * std::atan(x)
                - 0.5 * std::log(1.0 + x * x)
                );
    }
    // Segona antiderivada de l'arctangent, utilitzada per ADAA2.

    double DistortionProcess::arctanClipAntiderivada2(double x)
    {
        const double factor =
            2.0 / juce::MathConstants<double>::pi;

        const double xSquared = x * x;

        return factor *
            (
                0.5 * (xSquared - 1.0) * std::atan(x)
                + 0.5 * x
                * (1.0 - std::log(1.0 + xSquared))
                );
    }

    // El límit superior depèn del threshold i l'inferior es manté fix a -1.
    float DistortionProcess::asymmetricClip(float x)
    {
        if (x > threshold)
            return threshold;

        if (x < -1.0f)
            return -1.0f;

        return x;
    }

    // Primera antiderivada del clipping asimètric, utilitzada per ADAA1.
    double DistortionProcess::asymmetricClipAntiderivada(double x)
    {
        if (x < -1.0)
        {
            return -x - 0.5;
        }

        if (x > threshold)
        {
            return threshold * x
                - 0.5 * threshold * threshold;
        }

        return 0.5 * x * x;
    }
    // Segona antiderivada del clipping asimètric, utilitzada per ADAA2.

    double DistortionProcess::asymmetricClipAntiderivada2(double x)
    {
        const double thresholdSquared =
            threshold * threshold;

        const double thresholdCubed =
            thresholdSquared * threshold;

        if (x < -1.0)
        {
            return -0.5 * x * x
                - 0.5 * x
                - 1.0 / 6.0;
        }

        if (x > threshold)
        {
            return 0.5 * threshold * x * x
                - 0.5 * thresholdSquared * x
                + thresholdCubed / 6.0;
        }

        return (x * x * x) / 6.0;
    }
    // ADAA de primer ordre

    float DistortionProcess::processADAA1(
        float x,
        DistortionFunction function,
        AntiderivativeFunction antiderivative)
    {

        const double entradaActual =
            static_cast<double>(x);

        const double diferenciaEntrada =
            entradaActual - previousInput;

        double sortida = 0.0;

        if (std::abs(diferenciaEntrada) > adaaEpsilon)
        {
            sortida =
                (
                    (this->*antiderivative)(entradaActual)
                    - (this->*antiderivative)(previousInput)
                    )
                / diferenciaEntrada;
        }
        else
        {
            // el punt mig es calcula amb double.
            const double puntMig =
                0.5 * (entradaActual + previousInput);

            // La funció original continua treballant amb float.
            sortida = static_cast<double>(
                (this->*function)(
                    static_cast<float>(puntMig)));
        }

        // L'historial es guarda amb double.
        previousInput = entradaActual;

        // protecció davant d'un resultat NaN o infinit.
        if (!std::isfinite(sortida))
        {
            sortida = static_cast<double>(
                (this->*function)(x));
        }

        // només es converteix a float en retornar al buffer.
        return static_cast<float>(sortida);
    }
    // ADAA de segon ordre

    float DistortionProcess::processADAA2(
        float x,
        DistortionFunction function,
        AntiderivativeFunction antiderivative,
        AntiderivativeFunction secondAntiderivative)
    {
        // les tres mostres es representen amb double.
        const double entradaActual =
            static_cast<double>(x);

        const double entradaAnterior1 =
            previousInput;

        const double entradaAnterior2 =
            previousInput2;

        // Calcula una diferència dividida de F2.
        // els arguments, el denominador i el retorn són double.
        auto diferenciaDividida =
            [&](double a, double b) -> double
            {
                const double diferenciaEntrada =
                    a - b;

                if (std::abs(diferenciaEntrada) > adaaEpsilon)
                {
                    return
                        (
                            (this->*secondAntiderivative)(a)
                            - (this->*secondAntiderivative)(b)
                            )
                        / diferenciaEntrada;
                }

                // Quan a i b són pràcticament iguals,
                // la diferència dividida tendeix a F1.
                const double puntMig =
                    0.5 * (a + b);

                return (this->*antiderivative)(puntMig);
            };

        // totes les diferències es calculen amb double.
        const double diferenciaActual =
            diferenciaDividida(
                entradaActual,
                entradaAnterior1);

        const double diferenciaAnterior =
            diferenciaDividida(
                entradaAnterior1,
                entradaAnterior2);

        const double diferenciaExterior =
            entradaActual - entradaAnterior2;

        double sortida = 0.0;

        // Cas normal d'ADAA2.
        if (std::abs(diferenciaExterior) > adaaEpsilon)
        {
            sortida =
                2.0
                * (diferenciaActual - diferenciaAnterior)
                / diferenciaExterior;
        }
        else
        {
            // x[n] i x[n-2] són pràcticament iguals.
            const double diferenciaEntrada =
                entradaAnterior1 - entradaActual;

            if (std::abs(diferenciaEntrada) > adaaEpsilon)
            {
                sortida =
                    2.0
                    * (
                        diferenciaActual
                        - (this->*antiderivative)(entradaActual)
                        )
                    / diferenciaEntrada;
            }
            else
            {
                // Si les tres mostres són pràcticament iguals,
                // s'utilitza la funció original en el punt mig.
                const double puntMig =
                    (
                        entradaActual
                        + entradaAnterior1
                        + entradaAnterior2
                        )
                    / 3.0;

                sortida = static_cast<double>(
                    (this->*function)(
                        static_cast<float>(puntMig)));
            }
        }

        // s'actualitza l'historial mantenint precisió double.
        previousInput2 = entradaAnterior1;
        previousInput = entradaActual;

        // protecció davant de NaN o infinit.
        if (!std::isfinite(sortida))
        {
            sortida = static_cast<double>(
                (this->*function)(x));
        }

        // La sortida torna a float per escriure-la al buffer de JUCE.
        return static_cast<float>(sortida);
    }
}