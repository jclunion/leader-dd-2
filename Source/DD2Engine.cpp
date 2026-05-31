/*
  ==============================================================================

    DD2Engine.cpp
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Implémentation du moteur DSP pour l'émulation du Boss DD-2.

  ==============================================================================
*/

#include "DD2Engine.h"

DD2Engine::DD2Engine()
{
    // Initialisation des filtres et constantes temporelles par défaut
    reset();
}

void DD2Engine::prepareToPlay(double sr, int maxBlockSize)
{
    juce::ignoreUnused (maxBlockSize);
    sampleRate = sr;

    // Allocation de la mémoire tampon de retard pour supporter jusqu'à 1,5 seconde
    // Ceci s'adapte dynamiquement à n'importe quelle fréquence d'échantillonnage hôte (44.1kHz à 192kHz)
    int bufferSize = static_cast<int>(sampleRate * 1.5);
    
    for (int ch = 0; ch < 2; ++ch)
    {
        channels[ch].delayBuffer.assign(bufferSize, 0.0f);
        
        // Configuration des filtres anti-repliement (2 x 2nd ordre SVF = 4ème ordre)
        // Fréquence de coupure matérielle d'origine d'environ 7 kHz avec une pente raide.
        channels[ch].antiAliasFilter1.setSampleRate(sampleRate);
        channels[ch].antiAliasFilter1.setCutoff(7000.0f);
        channels[ch].antiAliasFilter1.setQ(0.7071f);
        channels[ch].antiAliasFilter1.setType(DD2Filter::Type::LowPass);

        channels[ch].antiAliasFilter2.setSampleRate(sampleRate);
        channels[ch].antiAliasFilter2.setCutoff(7000.0f);
        channels[ch].antiAliasFilter2.setQ(0.7071f);
        channels[ch].antiAliasFilter2.setType(DD2Filter::Type::LowPass);

        // Configuration des filtres de reconstruction (2 x 2nd ordre SVF = 4ème ordre)
        // Fréquence de coupure d'environ 7 kHz pour lisser les paliers de quantification de 12 bits à 32 kHz.
        channels[ch].reconstructFilter1.setSampleRate(sampleRate);
        channels[ch].reconstructFilter1.setCutoff(7000.0f);
        channels[ch].reconstructFilter1.setQ(0.7071f);
        channels[ch].reconstructFilter1.setType(DD2Filter::Type::LowPass);

        channels[ch].reconstructFilter2.setSampleRate(sampleRate);
        channels[ch].reconstructFilter2.setCutoff(7000.0f);
        channels[ch].reconstructFilter2.setQ(0.7071f);
        channels[ch].reconstructFilter2.setType(DD2Filter::Type::LowPass);

        // Filtre d'amortissement de boucle de rétroaction (Feedback Damping LPF)
        // Rend chaque répétition successivement plus sombre/chaude (coupure douce à 3.0 kHz, Q faible).
        channels[ch].feedbackDampFilter.setSampleRate(sampleRate);
        channels[ch].feedbackDampFilter.setCutoff(3000.0f);
        channels[ch].feedbackDampFilter.setQ(0.5f);
        channels[ch].feedbackDampFilter.setType(DD2Filter::Type::LowPass);

        // Enveloppes temporelles du compandeur NE570/571
        // Attaque rapide de 4 ms, relâchement de 10 ms
        channels[ch].compEnvelope.setSampleRate(sampleRate);
        channels[ch].compEnvelope.setAttackTime(4.0f);
        channels[ch].compEnvelope.setReleaseTime(10.0f);

        channels[ch].expEnvelope.setSampleRate(sampleRate);
        channels[ch].expEnvelope.setAttackTime(4.0f);
        channels[ch].expEnvelope.setReleaseTime(10.0f);

        // Réinitialisation des paramètres lissés
        channels[ch].smoothDelaySamples.reset(sampleRate, 0.05); // Réponse en 50ms pour un pitch-bending doux
        channels[ch].smoothFeedback.reset(sampleRate, 0.05);
        channels[ch].smoothELevel.reset(sampleRate, 0.05);

        // Décorrélation des graines aléatoires stéréo
        channels[ch].randomSeed = (ch == 0) ? 0x12345678 : 0x87654321;
    }

    reset();
}

void DD2Engine::reset()
{
    for (int ch = 0; ch < 2; ++ch)
    {
        channels[ch].reset();
    }
}

void DD2Engine::getModeDelayRange(int mode, float& minTimeSec, float& maxTimeSec) const
{
    switch (mode)
    {
        case 0: // Mode 1 : 12.5ms - 50ms (Short)
            minTimeSec = 0.0125f;
            maxTimeSec = 0.0500f;
            break;
        case 1: // Mode 2 : 50ms - 200ms (Medium)
            minTimeSec = 0.0500f;
            maxTimeSec = 0.2000f;
            break;
        case 2: // Mode 3 : 200ms - 800ms (Long)
            minTimeSec = 0.2000f;
            maxTimeSec = 0.8000f;
            break;
        case 3: // Mode 4 : HOLD (Même plage que Long, boucle infinie)
            minTimeSec = 0.2000f;
            maxTimeSec = 0.8000f;
            break;
        default:
            minTimeSec = 0.2000f;
            maxTimeSec = 0.8000f;
            break;
    }
}

void DD2Engine::process(juce::AudioBuffer<float>& buffer, 
                        float elevel, 
                        float feedback, 
                        float delayTimeParam, 
                        int mode, 
                        bool holdActive)
{
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    int bufferSize = static_cast<int>(channels[0].delayBuffer.size());

    // Calcul des plages de temps de retard pour définir les cibles lissées
    float minTimeSec, maxTimeSec;
    getModeDelayRange(mode, minTimeSec, maxTimeSec);
    float targetDelayTimeSec = minTimeSec + delayTimeParam * (maxTimeSec - minTimeSec);
    float targetDelaySamples = targetDelayTimeSec * static_cast<float>(sampleRate);

    // Limiter au nombre de canaux disponibles (max 2)
    int channelsToProcess = std::min(numChannels, 2);

    for (int ch = 0; ch < channelsToProcess; ++ch)
    {
        ChannelState& state = channels[ch];

        // Mettre à jour les cibles de lissage
        state.smoothDelaySamples.setTargetValue(targetDelaySamples);
        state.smoothFeedback.setTargetValue(feedback);
        state.smoothELevel.setTargetValue(elevel);

        // Gestion du déclenchement du mode HOLD numérique
        bool isHoldMode = (mode == 3);
        if (isHoldMode && holdActive)
        {
            if (!state.isHolding)
            {
                state.isHolding = true;
                state.holdStartWritePos = state.writeIndex;
                state.holdLengthSamples = static_cast<int>(state.smoothDelaySamples.getNextValue());
                if (state.holdLengthSamples < 10) state.holdLengthSamples = 10; // Sécurité anti-division par zéro
            }
        }
        else
        {
            state.isHolding = false;
        }

        float* channelData = buffer.getWritePointer(ch);

        for (int s = 0; s < numSamples; ++s)
        {
            float inputSample = channelData[s];

            // Obtenir les valeurs lissées pour cet échantillon
            float currentDelaySamples = state.smoothDelaySamples.getNextValue();
            float currentFeedback = state.smoothFeedback.getNextValue();
            float currentELevel = state.smoothELevel.getNextValue();

            float delayedSample = 0.0f;
            float quantizedSample = 0.0f;

            if (state.isHolding)
            {
                // En mode HOLD, nous bouclons parfaitement la mémoire numérique enregistrée
                // sans dégradation analogique cumulative (comme sur le matériel d'origine).
                int elapsedSamples = state.writeIndex - state.holdStartWritePos;
                if (elapsedSamples < 0) elapsedSamples += bufferSize;

                float loopOffset = std::fmod(static_cast<float>(elapsedSamples), static_cast<float>(state.holdLengthSamples));
                float relativeReadPos = static_cast<float>(state.holdStartWritePos) - static_cast<float>(state.holdLengthSamples) + loopOffset;
                
                delayedSample = readDelayCubic(state, static_cast<float>(state.writeIndex) - relativeReadPos);
                
                // On réécrit le signal numérique en boucle fermée dans la RAM
                state.delayBuffer[state.writeIndex] = delayedSample;
            }
            else
            {
                // Mode de fonctionnement Normal (Delay Classique)
                
                // 1. Lecture de la ligne de retard avec interpolation cubique premium
                delayedSample = readDelayCubic(state, currentDelaySamples);

                // --- Chemin Wet post-D/A : Expansion + Reconstruction ---
                // Expansion dynamique NE570 1:2
                float expEnvVal = state.expEnvelope.process(delayedSample);
                float expandedWet = delayedSample * (1.0f + compandBeta * expEnvVal);

                // Lissage des paliers (stair-steps) par le filtre de reconstruction 4ème ordre
                float reconstructedWet = state.reconstructFilter2.process(
                                            state.reconstructFilter1.process(expandedWet));

                // --- Boucle de rétroaction (Feedback Path) ---
                // Filtrage d'amortissement analogique
                float feedbackFiltered = state.feedbackDampFilter.process(reconstructedWet);

                // Sommation de l'entrée et de la boucle de rétroaction
                float feedbackSum = inputSample + feedbackFiltered * currentFeedback;

                // Écrêtage doux (Soft Clipping) type saturation analogique pour stabiliser l'auto-oscillation
                feedbackSum = std::tanh(feedbackSum);

                // --- Chemin de transmission Wet pré-A/D : Anti-repliement + Compression ---
                // Filtrage anti-aliasing raide à 7kHz
                float preFiltered = state.antiAliasFilter2.process(
                                        state.antiAliasFilter1.process(feedbackSum));

                // Compression dynamique NE570 2:1 en boucle fermée (Feedback Compressor)
                // Le gain est calculé en fonction de l'enveloppe du signal de SORTIE du compresseur.
                // Cela génère de magnifiques sursauts transitoires ("transient overshoot") et l'effet de respiration dynamique typique du DD-2.
                float lastCompEnv = state.compEnvelope.getCurrentEnvelope();
                float compGain = 1.0f / (1.0f + compandBeta * lastCompEnv);
                float compressedInput = preFiltered * compGain;

                // Mise à jour de l'enveloppe avec la sortie compressée actuelle pour le prochain échantillon
                state.compEnvelope.process(compressedInput);

                // --- Cœur Numérique : Décimation 32kHz + Quantification 12-bit ---
                state.sampleHoldCounter += 32000.0f / static_cast<float>(sampleRate);
                if (state.sampleHoldCounter >= 1.0f)
                {
                    state.sampleHoldCounter -= 1.0f;
                    // Prélèvement et maintien (Sample-and-Hold) + Quantification 12 bits émulée avec bruit et distorsion
                    state.currentHeldSample = quantize12Bit(compressedInput, state);
                }
                quantizedSample = state.currentHeldSample;

                // Écriture du signal quantifié final dans la ligne de retard
                state.delayBuffer[state.writeIndex] = quantizedSample;
            }

            // --- Étape Finale : Reconstruction post-D/A pour le signal Wet de sortie ---
            // Le signal retardé est de nouveau expansé et lissé pour la sortie wet
            float outExpEnvVal = state.expEnvelope.process(delayedSample);
            float outExpandedWet = delayedSample * (1.0f + compandBeta * outExpEnvVal);
            float finalWet = state.reconstructFilter2.process(
                                state.reconstructFilter1.process(outExpandedWet));

            // Mélange Wet/Dry type pédale Boss : signal Dry conservé à gain unitaire,
            // et E.Level contrôle le niveau injecté du signal Wet (0% à 100%).
            float outputMix = inputSample + finalWet * currentELevel;

            // Protection anti-écrêtage numérique brut
            channelData[s] = std::clamp(outputMix, -1.0f, 1.0f);

            // Incrémentation circulaire de l'index d'écriture
            state.writeIndex = (state.writeIndex + 1) % bufferSize;
        }
    }

    // Si le plugin reçoit du stéréo mais n'a qu'un canal traité (sécurité)
    if (numChannels > 2)
    {
        for (int ch = 2; ch < numChannels; ++ch)
        {
            buffer.clear(ch, 0, numSamples);
        }
    }
}
