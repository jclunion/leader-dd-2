/*
  ==============================================================================

    PluginProcessor.h
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Processeur audio principal JUCE pour l'émulation du Boss DD-2.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DD2Engine.h"

/**
 * @class DD2AudioProcessor
 * @brief Classe de processeur principal enveloppant le moteur DSP DD2Engine et gérant
 *        l'APVTS et l'interface avec le DAW.
 */
class DD2AudioProcessor  : public juce::AudioProcessor
{
public:
    DD2AudioProcessor();
    ~DD2AudioProcessor() override;

    // --- Surcharges standards de JUCE ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- Accesseurs publics pour l'éditeur (APVTS) ---
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    // --- Moteur de traitement DSP ---
    DD2Engine dspEngine;

    // --- Gestion des Paramètres ---
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Pointeurs atomiques rapides pour l'Audio Thread
    std::atomic<float>* eLevelParam   = nullptr;
    std::atomic<float>* feedbackParam = nullptr;
    std::atomic<float>* dTimeParam    = nullptr;
    std::atomic<float>* modeParam     = nullptr;
    std::atomic<float>* holdParam     = nullptr;

public:
    // --- Gestion des Presets d'Usine ---
    struct Preset
    {
        juce::String name;
        float elevel;
        float feedback;
        float dtime;
        int mode;
    };
    std::vector<Preset> factoryPresets;
    int currentProgramIndex = 0;

    // --- Gestion des Presets Utilisateur ---
    struct UserPreset
    {
        juce::String name;
        juce::File file;
    };
    std::vector<UserPreset> userPresets;

    void scanUserPresets();
    void saveUserPreset (const juce::String& presetName);
    void deleteUserPreset (int index);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DD2AudioProcessor)
};
