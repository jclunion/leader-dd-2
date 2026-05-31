/*
  ==============================================================================

    PluginEditor.h
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Interface graphique vintage style pédale Boss DD-2.
                 Contient le LookAndFeel personnalisé et l'animation de la LED.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// --- LookAndFeel Personnalisé pour l'esthétique Boss Stompbox ---
class BossLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BossLookAndFeel();
    ~BossLookAndFeel() override = default;

    /**
     * @brief Dessine un potentiomètre rotatif cylindrique vintage Boss DD-2.
     */
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    /**
     * @brief Personnalise le rendu des menus déroulants et boutons si nécessaire.
     */
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
};

// --- Éditeur Audio principal du Plugin ---
class DD2AudioProcessorEditor  : public juce::AudioProcessorEditor,
                                 public juce::Timer
{
public:
    DD2AudioProcessorEditor (DD2AudioProcessor&);
    ~DD2AudioProcessorEditor() override;

    // --- Surcharges de base de Component ---
    void paint (juce::Graphics&) override;
    void resized() override;

    // --- Surcharge de Timer pour l'animation ---
    void timerCallback() override;

private:
    DD2AudioProcessor& audioProcessor;

    // --- Style Visuel global ---
    BossLookAndFeel bossLookAndFeel;

    // --- Éléments de l'interface ---
    juce::Slider elevelSlider;
    juce::Slider feedbackSlider;
    juce::Slider dtimeSlider;
    juce::Slider modeSlider;        // Snaps rotatifs à 4 positions
    
    // Bouton de type commutateur au pied (Stomp Switch) métallique
    juce::TextButton stompSwitchButton;

    // Menu déroulant pour les presets d'usine et utilisateur
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;

    void updatePresetsList();
    void saveUserPreset();
    void deleteUserPreset();

    int lastProgramIndex = -1;

    // --- Étiquettes de texte rétro ---
    juce::Label elevelLabel;
    juce::Label feedbackLabel;
    juce::Label dtimeLabel;
    juce::Label modeLabel;

    // --- Attachements APVTS de JUCE (Gestion des paramètres sans effort) ---
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> elevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dtimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> holdAttachment;

    // --- Variables d'animation pour la LED rouge et le footswitch ---
    float ledPhase = 0.0f;
    float ledBrightness = 0.8f;
    bool isHoldActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DD2AudioProcessorEditor)
};
