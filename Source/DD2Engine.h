/*
  ==============================================================================

    DD2Engine.h
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Moteur de traitement DSP pour l'émulation du Boss DD-2.
                 Contient le compandeur NE570, la quantification 12-bit,
                 le rééchantillonnage à 32kHz, le filtre Simper SVF
                 et la logique du mode HOLD.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * @class DD2Filter
 * @brief Filtre d'intégration trapézoïdale de type Simper State Variable Filter (SVF).
 *        Extrêmement stable sous modulation et avec une réponse de phase excellente.
 */
class DD2Filter
{
public:
    enum class Type
    {
        LowPass,
        HighPass,
        BandPass,
        AllPass
    };

    DD2Filter() = default;
    ~DD2Filter() = default;

    void setSampleRate(double sr)
    {
        sampleRate = static_cast<float>(sr);
        updateCoefficients();
    }

    void setCutoff(float fc)
    {
        cutoff = std::clamp(fc, 10.0f, sampleRate * 0.49f);
        updateCoefficients();
    }

    void setQ(float newQ)
    {
        q = std::clamp(newQ, 0.01f, 40.0f);
        updateCoefficients();
    }

    void setType(Type newType)
    {
        type = newType;
    }

    void reset()
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    inline float process(float x)
    {
        // Intégration trapézoïdale standard de Simper SVF
        float v2 = (g * (x - s2) + s1) * a1;
        float v3 = s2 + g * v2;
        float v1 = x - k * v2 - v3;

        s1 = 2.0f * v2 - s1;
        s2 = 2.0f * v3 - s2;

        switch (type)
        {
            case Type::LowPass:  return v3;
            case Type::HighPass: return v1;
            case Type::BandPass: return v2;
            case Type::AllPass:  return x - 2.0f * k * v2;
        }
        return x;
    }

private:
    void updateCoefficients()
    {
        float gVal = std::tan(juce::MathConstants<float>::pi * cutoff / sampleRate);
        g = std::clamp(gVal, 0.0001f, 100.0f); // Protection contre les valeurs instables
        k = 1.0f / q;
        a1 = 1.0f / (1.0f + g * (g + k));
    }

    float sampleRate = 44100.0f;
    float cutoff = 1000.0f;
    float q = 0.7071f;
    Type type = Type::LowPass;

    float g = 0.0f, k = 0.0f, a1 = 0.0f;
    float s1 = 0.0f, s2 = 0.0f;
};

/**
 * @class DD2EnvelopeFollower
 * @brief Détecteur d'enveloppe avec temps d'attaque et de relâchement asymétriques.
 */
class DD2EnvelopeFollower
{
public:
    DD2EnvelopeFollower() = default;

    void setSampleRate(double sr)
    {
        sampleRate = static_cast<float>(sr);
        updateCoefficients();
    }

    void setAttackTime(float ms)
    {
        attackMs = ms;
        updateCoefficients();
    }

    void setReleaseTime(float ms)
    {
        releaseMs = ms;
        updateCoefficients();
    }

    void reset()
    {
        envelope = 0.0f;
    }

    inline float getCurrentEnvelope() const
    {
        return envelope;
    }

    inline float process(float x)
    {
        float absX = std::abs(x);
        float coeff = (absX > envelope) ? attackCoeff : releaseCoeff;
        envelope += coeff * (absX - envelope);
        return envelope;
    }

private:
    void updateCoefficients()
    {
        attackCoeff = 1.0f - std::exp(-1000.0f / (sampleRate * std::max(0.1f, attackMs)));
        releaseCoeff = 1.0f - std::exp(-1000.0f / (sampleRate * std::max(0.1f, releaseMs)));
    }

    float sampleRate = 44100.0f;
    float attackMs = 4.0f;   // NE570 compandeur typique
    float releaseMs = 10.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float envelope = 0.0f;
};

/**
 * @class DD2Engine
 * @brief Classe principale regroupant la logique DSP complète pour l'émulation du Boss DD-2.
 */
class DD2Engine
{
public:
    DD2Engine();
    ~DD2Engine() = default;

    /**
     * @brief Prépare le moteur DSP pour la lecture avec la fréquence d'échantillonnage et la taille de bloc.
     */
    void prepareToPlay(double sampleRate, int maxBlockSize);

    /**
     * @brief Réinitialise l'état interne (mémoire tampon, filtres, suiveurs d'enveloppe).
     */
    void reset();

    /**
     * @brief Traite un bloc audio stéréo.
     */
    void process(juce::AudioBuffer<float>& buffer, 
                 float elevel, 
                 float feedback, 
                 float delayTimeParam, 
                 int mode, 
                 bool holdActive);

    // (quantize12Bit a été déplacé sous ChannelState pour des raisons d'ordre de déclaration)

private:
    // --- Ligne de retard circulaire ---
    struct ChannelState
    {
        std::vector<float> delayBuffer;
        int writeIndex = 0;

        // Filtres anti-repliement (anti-aliasing) : 4ème ordre (2 x SVF 2nd ordre en série)
        DD2Filter antiAliasFilter1;
        DD2Filter antiAliasFilter2;

        // Filtres de reconstruction : 4ème ordre (2 x SVF 2nd ordre en série)
        DD2Filter reconstructFilter1;
        DD2Filter reconstructFilter2;

        // Filtre de feedback amorti (Feedback Damping)
        DD2Filter feedbackDampFilter;

        // Compandeur NE570
        DD2EnvelopeFollower compEnvelope;
        DD2EnvelopeFollower expEnvelope;

        // Rééchantillonnage 32kHz
        float sampleHoldCounter = 0.0f;
        float currentHeldSample = 0.0f;

        // Valeurs lissées
        juce::LinearSmoothedValue<float> smoothDelaySamples { 0.0f };
        juce::LinearSmoothedValue<float> smoothFeedback { 0.0f };
        juce::LinearSmoothedValue<float> smoothELevel { 0.0f };

        // État de boucle HOLD
        bool isHolding = false;
        int holdStartWritePos = 0;
        int holdLengthSamples = 0;

        // Générateur de bruit LCG pour le dither 12-bit
        uint32_t randomSeed = 0x12345678;

        void reset()
        {
            std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
            writeIndex = 0;
            antiAliasFilter1.reset();
            antiAliasFilter2.reset();
            reconstructFilter1.reset();
            reconstructFilter2.reset();
            feedbackDampFilter.reset();
            compEnvelope.reset();
            expEnvelope.reset();
            sampleHoldCounter = 0.0f;
            currentHeldSample = 0.0f;
            isHolding = false;
            holdStartWritePos = 0;
            holdLengthSamples = 0;
            // Graine de départ par défaut
            randomSeed = 0x12345678;
        }
    };

    /**
     * @brief Quantification 12-bit avec émulation du bruit thermique ADC (LSB Toggle)
     *        et distorsion de croisement (MSB Glitch) typique des puces vintage.
     */
    inline float quantize12Bit(float input, ChannelState& state)
    {
        float clamped = std::clamp(input, -1.0f, 1.0f);

        // 1. Émulation du bruit thermique d'entrée ADC (environ -65 dB)
        // Fait osciller le LSB de manière aléatoire
        state.randomSeed = state.randomSeed * 196314165 + 907633515;
        float noise = (static_cast<float>(static_cast<int32_t>(state.randomSeed)) / 2147483647.0f) * 0.0006f;
        clamped += noise;

        // 2. Émulation de la distorsion de croisement (MSB Glitch)
        // Se produit autour de zéro lorsque le bit de poids fort commute sur les DACs 12-bit (ex: PCM53)
        if (std::abs(clamped) < 0.07f)
        {
            clamped += 0.0035f * std::sin(clamped * (juce::MathConstants<float>::pi / 0.07f));
        }

        // 3. Quantification 12-bit stricte
        float scaled = clamped * 2048.0f;
        float quantized = std::round(scaled);
        return quantized / 2048.0f;
    }

    /**
     * @brief Obtient la plage de temps de retard (en secondes) pour le mode sélectionné.
     */
    void getModeDelayRange(int mode, float& minTimeSec, float& maxTimeSec) const;

    /**
     * @brief Interpolation cubique d'Hermite pour la lecture de la ligne de retard.
     */
    inline float readDelayCubic(const ChannelState& state, float delaySamples) const
    {
        int bufferSize = static_cast<int>(state.delayBuffer.size());
        
        float readPos = static_cast<float>(state.writeIndex) - delaySamples;
        while (readPos < 0.0f) readPos += static_cast<float>(bufferSize);

        int i1 = static_cast<int>(readPos);
        float frac = readPos - static_cast<float>(i1);

        int i0 = (i1 - 1 + bufferSize) % bufferSize;
        int i2 = (i1 + 1) % bufferSize;
        int i3 = (i1 + 2) % bufferSize;

        i1 = i1 % bufferSize;

        float y0 = state.delayBuffer[i0];
        float y1 = state.delayBuffer[i1];
        float y2 = state.delayBuffer[i2];
        float y3 = state.delayBuffer[i3];

        // Formule cubique d'Hermite
        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

    double sampleRate = 44100.0;
    ChannelState channels[2];
    
    // Constante de compandeur β (facteur d'amplification/atténuation d'enveloppe)
    const float compandBeta = 4.0f; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DD2Engine)
};
