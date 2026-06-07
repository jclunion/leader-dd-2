/*
  ==============================================================================

    PluginEditor.cpp
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Implémentation de l'interface graphique pour le Leader DD-2.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ==============================================================================
// --- Implémentation du LookAndFeel Leader ---
// ==============================================================================
LeaderLookAndFeel::LeaderLookAndFeel()
{
    // Définir des couleurs globales par défaut
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
}

void LeaderLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    juce::ignoreUnused (slider);

    float radius = std::min (width, height) * 0.44f;
    float cx = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
    float cy = static_cast<float> (y) + static_cast<float> (height) * 0.5f;

    // --- 1. Corps principal du potentiomètre (Cylindre noir-gris) ---
    juce::Colour greyBody = juce::Colour (0xFF2F2F33);
    juce::Colour greyBodyDark = juce::Colour (0xFF141416);
    
    // Ombre portée sous le bouton
    g.setColour (juce::Colour (0x99000000));
    g.fillEllipse (cx - radius + 1.0f, cy - radius + 2.0f, radius * 2.0f, radius * 2.0f);

    g.setGradientFill (juce::ColourGradient::vertical (greyBody, cy - radius, greyBodyDark, cy + radius));
    g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    // Bevel/Ombre extérieure
    g.setColour (juce::Colour (0xFF0B0B0C));
    g.drawEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.8f);

    // --- 2. Capuchon central en aluminium brossé ---
    float capRadius = radius * 0.52f;
    juce::Colour capLight = juce::Colour (0xFFFDFDFD);
    juce::Colour capDark = juce::Colour (0xFF99999C);
    
    // Gradient linéaire incliné pour émuler le reflet de l'aluminium brossé
    g.setGradientFill (juce::ColourGradient (capLight, cx - capRadius * 0.6f, cy - capRadius * 0.6f,
                                               capDark, cx + capRadius * 0.8f, cy + capRadius * 0.8f, false));
    g.fillEllipse (cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Fine rainure interne sur le capuchon
    g.setColour (juce::Colour (0x33FFFFFF));
    g.drawEllipse (cx - capRadius * 0.8f, cy - capRadius * 0.8f, capRadius * 1.6f, capRadius * 1.6f, 0.8f);
    g.setColour (juce::Colour (0x55000000));
    g.drawEllipse (cx - capRadius, cy - capRadius, capRadius * 2.0f, capRadius * 2.0f, 0.5f);

    // --- 3. Ligne d'indication blanche (Indicateur de position) ---
    float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    
    // La ligne part du bord du capuchon jusqu'au bord extérieur du corps
    float rx1 = cx + (capRadius + 1.0f) * std::sin (angle);
    float ry1 = cy - (capRadius + 1.0f) * std::cos (angle);
    float rx2 = cx + (radius - 1.0f) * std::sin (angle);
    float ry2 = cy - (radius - 1.0f) * std::cos (angle);

    g.setColour (juce::Colours::white);
    g.drawLine (rx1, ry1, rx2, ry2, 2.5f);
}

void LeaderLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused (backgroundColour);

    if (button.getName() == "Footswitch")
    {
        // Rendu interactif du Commutateur de pied (Footswitch)
        bool isDown = shouldDrawButtonAsDown || button.getToggleState();
        
        // Décalage visuel en pixel pour un enfoncement 3D saisissant
        float offset = isDown ? 3.0f : 0.0f;

        // Rectangle de base dans les coordonnées locales du bouton
        float w = static_cast<float> (button.getWidth());
        float h = static_cast<float> (button.getHeight());

        // Fond métallique sombre sous le footswitch
        g.setColour (juce::Colour (0xFF162130));
        g.fillRoundedRectangle (0.0f, 0.0f, w, h, 6.0f);

        // Ombre portée sous le switch s'il est relevé
        if (!isDown)
        {
            g.setColour (juce::Colour (0x7F000000));
            g.fillRoundedRectangle (0.0f, 3.0f, w, h - 3.0f, 6.0f);
        }

        // Le corps principal du footswitch (qui s'enfonce)
        juce::Rectangle<float> switchFace (0.0f, offset, w, h - 3.0f);
        juce::Colour pedalBlue = juce::Colour (0xFF1E2D42);
        juce::Colour pedalBlueLight = juce::Colour (0xFF263954);
        
        g.setGradientFill (juce::ColourGradient::vertical (pedalBlueLight, switchFace.getY(), pedalBlue, switchFace.getBottom()));
        g.fillRoundedRectangle (switchFace, 6.0f);

        // Effet d'assombrissement si enfoncé
        if (isDown)
        {
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRoundedRectangle (switchFace, 6.0f);
        }

        // Bordure en plastique noir du footswitch
        g.setColour (juce::Colour (0xFF0E141D));
        g.drawRoundedRectangle (switchFace, 6.0f, 2.0f);

        // Plaque de protection en caoutchouc antidérapant (Rubber Pad)
        juce::Rectangle<float> rubberPad (10.0f, switchFace.getY() + 10.0f, w - 20.0f, h - 65.0f);
        juce::Colour rubberGrey = juce::Colour (0xFF17171A);
        juce::Colour rubberBlack = juce::Colour (0xFF0A0A0B);
        g.setGradientFill (juce::ColourGradient::vertical (rubberGrey, rubberPad.getY(), rubberBlack, rubberPad.getBottom()));
        g.fillRoundedRectangle (rubberPad, 4.0f);

        if (isDown)
        {
            g.setColour (juce::Colours::black.withAlpha (0.15f));
            g.fillRoundedRectangle (rubberPad, 4.0f);
        }

        // Rainures horizontales sur le caoutchouc antidérapant
        g.setColour (juce::Colour (0xFF050506));
        float rubberStep = rubberPad.getHeight() / 10.0f;
        for (int i = 1; i < 10; ++i)
        {
            float ry = rubberPad.getY() + static_cast<float> (i) * rubberStep;
            g.drawHorizontalLine (static_cast<int> (ry), rubberPad.getX() + 4.0f, rubberPad.getRight() - 4.0f);
        }

        // Texte sur la pédale de commutation
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions ("Arial", 12.0f, juce::Font::bold)));
        g.drawText ("PRESS PEDAL TO LOOP", rubberPad.withY (rubberPad.getY() + 35.0f).withHeight (25.0f), juce::Justification::centred);

        g.setFont (juce::Font (juce::FontOptions ("Arial", 18.0f, juce::Font::bold)));
        g.drawText ("HOLD", rubberPad.withY (rubberPad.getY() + 65.0f).withHeight (30.0f), juce::Justification::centred);

        // Vis de serrage argentée au bas du footswitch (Boss Screw)
        float screwX = w * 0.5f;
        float screwY = switchFace.getBottom() - 22.0f;

        // 1. Cavité extérieure (Trou dans le métal)
        g.setColour (juce::Colour (0xFF080809));
        g.fillEllipse (screwX - 11.0f, screwY - 11.0f, 22.0f, 22.0f);

        // 2. Corps de la vis métallique argentée
        juce::Colour screwLight = juce::Colour (0xFFCCCCCC);
        juce::Colour screwDark = juce::Colour (0xFF555555);
        g.setGradientFill (juce::ColourGradient::vertical (screwLight, screwY - 9.0f, screwDark, screwY + 9.0f));
        g.fillEllipse (screwX - 9.0f, screwY - 9.0f, 18.0f, 18.0f);

        // 3. Fente de tournevis au centre de la vis
        g.setColour (juce::Colour (0xFF1A1A1A));
        g.fillRect (screwX - 6.0f, screwY - 1.5f, 12.0f, 3.0f);
        g.fillRect (screwX - 1.5f, screwY - 6.0f, 3.0f, 12.0f);

        // 4. Ombre intérieure (Inner Shadow) pour l'effet de renfoncement 3D dans le boîtier
        juce::ColourGradient innerShadow (juce::Colours::black.withAlpha (0.6f), screwX - 9.0f, screwY - 9.0f,
                                          juce::Colours::transparentBlack, screwX + 2.0f, screwY + 2.0f, true);
        g.setGradientFill (innerShadow);
        g.fillEllipse (screwX - 9.0f, screwY - 9.0f, 18.0f, 18.0f);

        // 5. Bordure de la cavité / Rondelle en caoutchouc noir
        g.setColour (juce::Colour (0xFF020203));
        g.drawEllipse (screwX - 11.0f, screwY - 11.0f, 22.0f, 22.0f, 1.2f);
        g.drawEllipse (screwX - 9.0f, screwY - 9.0f, 18.0f, 18.0f, 0.8f);
    }
    else if (button.getName() == "SavePreset" || button.getName() == "DeletePreset")
    {
        auto bounds = button.getLocalBounds().toFloat();
        juce::Colour fillColour = juce::Colours::transparentBlack;
        juce::Colour borderColour = juce::Colour (0x33FFFFFF);
        
        if (shouldDrawButtonAsDown)
        {
            fillColour = juce::Colours::white.withAlpha (0.15f);
            borderColour = juce::Colours::white.withAlpha (0.6f);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            fillColour = juce::Colours::white.withAlpha (0.08f);
            borderColour = juce::Colours::white.withAlpha (0.4f);
        }
        
        g.setColour (fillColour);
        g.fillRoundedRectangle (bounds, 3.0f);
        
        g.setColour (borderColour);
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    }
}

juce::Font LeaderLookAndFeel::getTextButtonFont (juce::TextButton& button, int buttonHeight)
{
    juce::ignoreUnused (buttonHeight);
    if (button.getName() == "SavePreset" || button.getName() == "DeletePreset")
    {
        return juce::Font (juce::FontOptions ("Arial", 10.0f, juce::Font::bold));
    }
    return juce::Font (juce::FontOptions ("Arial", 12.0f, juce::Font::bold));
}

// ==============================================================================
// --- Implémentation de DD2AudioProcessorEditor ---
// ==============================================================================
DD2AudioProcessorEditor::DD2AudioProcessorEditor (DD2AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Définir la taille de la pédale (ratio vertical classique Boss)
    setSize (400, 550);

    // Application du Look and Feel personnalisé aux potentiomètres
    setLookAndFeel (&leaderLookAndFeel);

    // --- Configuration du curseur E.LEVEL ---
    elevelSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    elevelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    elevelSlider.setRange (0.0f, 1.0f, 0.01f);
    addAndMakeVisible (elevelSlider);

    elevelLabel.setText ("E.LEVEL", juce::dontSendNotification);
    elevelLabel.setFont (juce::Font (juce::FontOptions ("Arial", 10.0f, juce::Font::bold)));
    elevelLabel.setJustificationType (juce::Justification::centred);
    elevelLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFD1D5DB));
    addAndMakeVisible (elevelLabel);

    // --- Configuration du curseur F.BACK ---
    feedbackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    feedbackSlider.setRange (0.0f, 1.15f, 0.01f);
    addAndMakeVisible (feedbackSlider);

    feedbackLabel.setText ("F.BACK", juce::dontSendNotification);
    feedbackLabel.setFont (juce::Font (juce::FontOptions ("Arial", 10.0f, juce::Font::bold)));
    feedbackLabel.setJustificationType (juce::Justification::centred);
    feedbackLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFD1D5DB));
    addAndMakeVisible (feedbackLabel);

    // --- Configuration du curseur D.TIME ---
    dtimeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    dtimeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dtimeSlider.setRange (0.0f, 1.0f, 0.001f);
    addAndMakeVisible (dtimeSlider);

    dtimeLabel.setText ("D.TIME", juce::dontSendNotification);
    dtimeLabel.setFont (juce::Font (juce::FontOptions ("Arial", 10.0f, juce::Font::bold)));
    dtimeLabel.setJustificationType (juce::Justification::centred);
    dtimeLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFD1D5DB));
    addAndMakeVisible (dtimeLabel);

    // --- Configuration du curseur MODE ---
    modeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    modeSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modeSlider.setRange (0, 3, 1);
    addAndMakeVisible (modeSlider);

    modeLabel.setText ("MODE / RANGE", juce::dontSendNotification);
    modeLabel.setFont (juce::Font (juce::FontOptions ("Arial", 10.0f, juce::Font::bold)));
    modeLabel.setJustificationType (juce::Justification::centred);
    modeLabel.setColour (juce::Label::textColourId, juce::Colour (0xFFD1D5DB));
    addAndMakeVisible (modeLabel);

    // --- Configuration du bouton Stomp Switch ---
    stompSwitchButton.setName ("Footswitch");
    stompSwitchButton.setButtonText ("");
    stompSwitchButton.setClickingTogglesState (true);
    addAndMakeVisible (stompSwitchButton);

    // --- Configuration du Menu Déroulant des Presets ---
    presetComboBox.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xFF2B3F5C));
    presetComboBox.setColour (juce::ComboBox::textColourId, juce::Colour (0xFFE5E7EB));
    presetComboBox.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xFF3A5478));
    presetComboBox.setColour (juce::ComboBox::arrowColourId, juce::Colour (0xFFF39C12));
    
    presetComboBox.onChange = [this]()
    {
        int index = presetComboBox.getSelectedItemIndex();
        audioProcessor.setCurrentProgram (index);
        updatePresetsList(); // Met à jour l'état activé du bouton de suppression
    };
    addAndMakeVisible (presetComboBox);

    // Bouton de sauvegarde de preset
    savePresetButton.setName ("SavePreset");
    savePresetButton.setButtonText ("SAVE");
    savePresetButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFFE5E7EB));
    savePresetButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    savePresetButton.onClick = [this]() { saveUserPreset(); };
    addAndMakeVisible (savePresetButton);

    // Bouton de suppression de preset
    deletePresetButton.setName ("DeletePreset");
    deletePresetButton.setButtonText ("DEL");
    deletePresetButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFFE5E7EB));
    deletePresetButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    deletePresetButton.onClick = [this]() { deleteUserPreset(); };
    addAndMakeVisible (deletePresetButton);

    // Initialisation et chargement de la liste
    updatePresetsList();

    // --- Liaison APVTS ---
    elevelAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "elevel", elevelSlider);
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "feedback", feedbackSlider);
    dtimeAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "dtime", dtimeSlider);
    modeAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.getAPVTS(), "mode", modeSlider);
    holdAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.getAPVTS(), "hold", stompSwitchButton);

    // Démarrage du Timer à 60 FPS pour l'animation fluide de la LED et de l'enfoncement tactile
    startTimerHz (60);
}

DD2AudioProcessorEditor::~DD2AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

// ==============================================================================
// --- Rendu Graphique Principal (paint) ---
// ==============================================================================
void DD2AudioProcessorEditor::paint (juce::Graphics& g)
{
    // --- 1. Texture Métallique du Châssis Boss ---
    juce::Colour pedalBlue = juce::Colour (0xFF1E2D42); // Vintage Boss Blue/Grey
    juce::Colour pedalBlueLight = juce::Colour (0xFF263954);
    juce::Colour pedalBlueDark = juce::Colour (0xFF121C29);
    
    // Dégradé vertical pour un look incurvé 3D métallique réaliste
    g.setGradientFill (juce::ColourGradient::vertical (pedalBlueLight, 0.0f, pedalBlueDark, static_cast<float> (getHeight())));
    g.fillAll();

    // Effet biseau/bordures lumineuses sur le pourtour extérieur
    g.setColour (juce::Colour (0x22FFFFFF));
    g.drawRect (juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (getWidth()), static_cast<float> (getHeight())), 1.5f);
    g.setColour (juce::Colour (0x44000000));
    g.drawRect (juce::Rectangle<float> (1.0f, 1.0f, static_cast<float> (getWidth() - 2), static_cast<float> (getHeight() - 2)), 1.0f);

    // --- 2. Plaque de Contrôle supérieure en plastique bleu-roi ---
    juce::Rectangle<float> controlPanel (15.0f, 60.0f, 370.0f, 132.0f);
    juce::Colour panelBlue = juce::Colour (0xFF0E1A2B); // Plaque foncée
    juce::Colour panelBlueBorder = juce::Colour (0xFF2A4266);
    
    g.setColour (panelBlue);
    g.fillRoundedRectangle (controlPanel, 4.0f);
    g.setColour (panelBlueBorder);
    g.drawRoundedRectangle (controlPanel, 4.0f, 1.5f);

    // Séparateurs verticaux fins 3D ("bays") entre les contrôles sur la plaque
    float step = controlPanel.getWidth() / 4.0f;
    for (int i = 1; i < 4; ++i)
    {
        float splitX = controlPanel.getX() + static_cast<float> (i) * step;
        // Ligne d'ombre
        g.setColour (juce::Colour (0xFF080F1A));
        g.drawVerticalLine (static_cast<int> (splitX), controlPanel.getY() + 2.0f, controlPanel.getBottom() - 2.0f);
        // Ligne de lumière
        g.setColour (juce::Colour (0xFF263E61));
        g.drawVerticalLine (static_cast<int> (splitX) + 1, controlPanel.getY() + 2.0f, controlPanel.getBottom() - 2.0f);
    }

    // --- 3. Bandeau Sérigraphié Inférieur sur la plaque de contrôle ---
    juce::Rectangle<float> stripeRect (16.0f, 150.0f, 368.0f, 40.0f);
    g.setColour (juce::Colour (0xFF1F3556)); // Bandeau bleu plus clair
    g.fillRect (stripeRect);
    
    g.setColour (juce::Colour (0xFF3A5885));
    g.drawHorizontalLine (150, 16.0f, 384.0f);

    // --- 4. Rendu de la LED d'état Rouge animée (CHECK) ---
    float ledX = 200.0f;
    float ledY = 30.0f;

    // Bezel chrome externe
    juce::Path chromeBezel;
    chromeBezel.addCentredArc (ledX, ledY, 8.0f, 8.0f, 0.0f, 0.0f, juce::MathConstants<float>::twoPi, true);
    juce::ColourGradient chromeGrad (juce::Colours::white, ledX - 6.0f, ledY - 6.0f,
                                     juce::Colour (0xFF666666), ledX + 6.0f, ledY + 6.0f, false);
    g.setGradientFill (chromeGrad);
    g.fillPath (chromeBezel);
    
    // Bordure argentée pour faire ressortir le chrome
    g.setColour (juce::Colour (0xFF999999));
    g.drawEllipse (ledX - 8.0f, ledY - 8.0f, 16.0f, 16.0f, 1.5f);

    // Anneau de caoutchouc intérieur sombre
    g.setColour (juce::Colour (0xFF111111));
    g.fillEllipse (ledX - 5.5f, ledY - 5.5f, 11.0f, 11.0f);

    // Couleur dynamique de la LED en fonction de l'oscillation et de l'état HOLD
    juce::Colour ledColor;
    if (isHoldActive)
    {
        ledColor = juce::Colour::fromFloatRGBA (1.0f, 0.05f, 0.05f, 1.0f);
    }
    else
    {
        ledColor = juce::Colour::fromFloatRGBA (0.2f + 0.8f * ledBrightness, 0.0f, 0.0f, 1.0f);
    }

    g.setColour (ledColor);
    g.fillEllipse (ledX - 4.0f, ledY - 4.0f, 8.0f, 8.0f);

    // Halo lumineux / Glow
    if (ledBrightness > 0.05f || isHoldActive)
    {
        float currentHalo = 14.0f * (isHoldActive ? 1.0f : ledBrightness);
        juce::ColourGradient glow (ledColor.withAlpha (0.4f * (isHoldActive ? 1.0f : ledBrightness)), ledX, ledY,
                                   juce::Colours::transparentBlack, ledX + currentHalo, ledY + currentHalo, true);
        g.setGradientFill (glow);
        g.fillEllipse (ledX - currentHalo, ledY - currentHalo, currentHalo * 2.0f, currentHalo * 2.0f);

        // Point brillant spéculaire
        g.setColour (juce::Colour (0xCCFFFFFF));
        g.fillEllipse (ledX - 1.5f, ledY - 2.0f, 1.5f, 1.5f);
    }

    // Étiquette "CHECK" pour la LED
    g.setColour (juce::Colour (0xFFE5E7EB));
    g.setFont (juce::Font (juce::FontOptions ("Arial", 9.0f, juce::Font::bold)));
    g.drawText ("CHECK", 150, 6, 100, 12, juce::Justification::centred);

    // --- 5. Graduations autour du bouton MODE (4 positions) ---
    float modeCx = 337.5f;
    float modeCy = 108.0f;
    float startAngle = 1.25f * juce::MathConstants<float>::pi; // 225 deg (Bottom-Left)
    float endAngle = 2.75f * juce::MathConstants<float>::pi;   // 135 deg (Bottom-Right)

    // Petit cercle de repère orange pour chaque position
    g.setColour (juce::Colour (0xFFF39C12)); // Orange Boss
    float tickRadius = 26.0f;
    for (int i = 0; i < 4; ++i)
    {
        float angle = startAngle + (static_cast<float> (i) / 3.0f) * (endAngle - startAngle);
        float tx = modeCx + tickRadius * std::sin (angle);
        float ty = modeCy - tickRadius * std::cos (angle);
        g.fillEllipse (tx - 1.5f, ty - 1.5f, 3.0f, 3.0f);
    }

    // On dessine les textes dans des zones rectangulaires fixes et compactes pour éviter les chevauchements
    g.setFont (juce::Font (juce::FontOptions ("Arial", 7.5f, juce::Font::bold)));
    g.setColour (juce::Colour (0xFFD1D5DB));

    // Position 0 : 12.5-50 (Bottom-Left)
    g.drawText ("12.5-50", 285, 133, 32, 10, juce::Justification::centredRight);
    
    // Position 1 : 50-200 (Top-Left)
    g.drawText ("50-200", 285, 73, 32, 10, juce::Justification::centredRight);

    // Position 2 : 200-800 (Top-Right)
    g.drawText ("200-800", 358, 73, 32, 10, juce::Justification::centredLeft);

    // Position 3 : HOLD (Bottom-Right)
    g.drawText ("HOLD", 358, 133, 32, 10, juce::Justification::centredLeft);


    // Affichage textuel dynamique du mode actif sous le bouton MODE
    int activeMode = static_cast<int> (modeSlider.getValue());
    juce::String modeString;
    if (activeMode == 0)      modeString = "Short Delay";
    else if (activeMode == 1) modeString = "Medium Delay";
    else if (activeMode == 2) modeString = "Long Delay";
    else if (activeMode == 3) modeString = "HOLD Loop";

    g.setColour (juce::Colour (0xFFF39C12)); // Orange
    g.setFont (juce::Font (juce::FontOptions ("Arial", 9.5f, juce::Font::bold)));
    g.drawText (modeString, 299, 172, 76, 14, juce::Justification::centred);

    // --- 6. Textes et Logos Rétro Industriels au centre ---
    // Logo Géant "LEADER"
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions ("Arial Black", 38.0f, juce::Font::bold | juce::Font::italic)));
    g.drawText ("LEADER", 35, 205, 200, 50, juce::Justification::left);

    // Logo du modèle "Digital Delay" en orange Boss classique
    g.setColour (juce::Colour (0xFFF39C12));
    g.setFont (juce::Font (juce::FontOptions ("Arial", 16.0f, juce::Font::bold)));
    g.drawText ("Digital Delay", 35, 255, 250, 22, juce::Justification::left);

    // "DD-2" en gras blanc sous "Digital Delay"
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions ("Arial Black", 24.0f, juce::Font::bold)));
    g.drawText ("DD-2", 35, 277, 150, 30, juce::Justification::left);
}

// ==============================================================================
// --- Positionnement des Composants (resized) ---
// ==============================================================================
void DD2AudioProcessorEditor::resized()
{
    // Disposer les 4 potentiomètres de manière équilibrée sur la plaque supérieure
    // Plaque de contrôle x=15, y=60, w=370, h=120
    float startX = 22.0f;
    float stepWidth = 370.0f / 4.0f;
    float sliderWidth = 76.0f;
    float sliderHeight = 76.0f;
    float labelHeight = 16.0f;

    // Curseur E.LEVEL
    elevelSlider.setBounds (static_cast<int> (startX), 70, static_cast<int> (sliderWidth), static_cast<int> (sliderHeight));
    elevelLabel.setBounds (static_cast<int> (startX - 2.0f), 154, static_cast<int> (sliderWidth + 4.0f), static_cast<int> (labelHeight));

    // Curseur F.BACK
    feedbackSlider.setBounds (static_cast<int> (startX + stepWidth), 70, static_cast<int> (sliderWidth), static_cast<int> (sliderHeight));
    feedbackLabel.setBounds (static_cast<int> (startX + stepWidth - 2.0f), 154, static_cast<int> (sliderWidth + 4.0f), static_cast<int> (labelHeight));

    // Curseur D.TIME
    dtimeSlider.setBounds (static_cast<int> (startX + 2.0f * stepWidth), 70, static_cast<int> (sliderWidth), static_cast<int> (sliderHeight));
    dtimeLabel.setBounds (static_cast<int> (startX + 2.0f * stepWidth - 2.0f), 154, static_cast<int> (sliderWidth + 4.0f), static_cast<int> (labelHeight));

    // Curseur MODE
    modeSlider.setBounds (static_cast<int> (startX + 3.0f * stepWidth), 70, static_cast<int> (sliderWidth), static_cast<int> (sliderHeight));
    modeLabel.setBounds (static_cast<int> (startX + 3.0f * stepWidth - 2.0f), 154, static_cast<int> (sliderWidth + 4.0f), static_cast<int> (labelHeight));

    // Le bouton stomp switch couvre toute la zone physique du footswitch inférieur
    stompSwitchButton.setBounds (25, 315, 350, 215);

    // Positionnement des contrôles de presets (Gauche et Droite) pour laisser la LED centrale visible
    presetComboBox.setBounds (15, 18, 120, 24);
    savePresetButton.setBounds (275, 18, 50, 24);
    deletePresetButton.setBounds (335, 18, 50, 24);
}

// ==============================================================================
// --- Animation à 60 Hz (timerCallback) ---
// ==============================================================================
void DD2AudioProcessorEditor::timerCallback()
{
    // 1. Lire l'état du paramètre HOLD actuel de l'APVTS de façon asynchrone
    isHoldActive = audioProcessor.getAPVTS().getRawParameterValue ("hold")->load() > 0.5f;

    // 2. Déterminer la vitesse de clignotement de la LED en fonction de dtime et du mode
    float dtimeVal = static_cast<float> (dtimeSlider.getValue());
    int modeVal = static_cast<int> (modeSlider.getValue());

    // Calculer le temps de retard actuel
    float minSec = 0.2f;
    float maxSec = 0.8f;
    
    if (modeVal == 0)      { minSec = 0.0125f; maxSec = 0.0500f; }
    else if (modeVal == 1) { minSec = 0.0500f; maxSec = 0.2000f; }
    else if (modeVal == 2) { minSec = 0.2000f; maxSec = 0.8000f; }
    else if (modeVal == 3) { minSec = 0.2000f; maxSec = 0.8000f; } // HOLD

    float currentDelaySec = minSec + dtimeVal * (maxSec - minSec);

    // Ajuster l'incrémentation de phase (Timer appelé toutes les 16.6ms)
    float timerSec = 1.0f / 60.0f;
    
    // Le rythme de pulsation s'adapte à la fréquence du délai
    ledPhase += (juce::MathConstants<float>::twoPi / currentDelaySec) * timerSec;
    if (ledPhase > juce::MathConstants<float>::twoPi)
    {
        ledPhase -= juce::MathConstants<float>::twoPi;
    }

    if (modeVal == 3)
    {
        // En mode HOLD :
        // Si HOLD est actif (bouclage infini) -> LED allumée en continu
        // Si HOLD est inactif (écoute standard) -> LED faiblement éclairée
        ledBrightness = isHoldActive ? 1.0f : 0.15f;
    }
    else
    {
        // En mode normal :
        // Clignotement périodique sinus
        float sinVal = std::sin (ledPhase);
        ledBrightness = 0.15f + 0.85f * (0.5f + 0.5f * sinVal);
    }

    // Synchronisation bidirectionnelle du menu déroulant avec les presets du processeur
    // Correction de la condition de concurrence : on n'écrase la sélection que si l'index du programme a RÉELLEMENT changé à l'extérieur.
    int currentProg = audioProcessor.getCurrentProgram();
    if (lastProgramIndex != currentProg)
    {
        lastProgramIndex = currentProg;
        updatePresetsList();
    }

    // Provoquer le rafraîchissement graphique de toute la pédale à 60 FPS
    repaint();
}

// --- Méthodes auxiliaires de gestion de la liste des presets ---
void DD2AudioProcessorEditor::updatePresetsList()
{
    presetComboBox.clear (juce::dontSendNotification);
    
    // Remplir avec les presets d'usine et utilisateur
    for (int i = 0; i < audioProcessor.getNumPrograms(); ++i)
    {
        presetComboBox.addItem (audioProcessor.getProgramName (i), i + 1);
    }
    
    int currentProg = audioProcessor.getCurrentProgram();
    presetComboBox.setSelectedItemIndex (currentProg, juce::dontSendNotification);
    lastProgramIndex = currentProg;
    
    // Le bouton DELETE n'est actif que pour les presets utilisateurs (index >= 5)
    deletePresetButton.setEnabled (currentProg >= 5);
}

void DD2AudioProcessorEditor::saveUserPreset()
{
    auto* alert = new juce::AlertWindow ("Sauvegarder Preset", 
                                         "Entrez le nom de votre preset utilisateur :", 
                                         juce::AlertWindow::QuestionIcon);
    alert->addTextEditor ("presetName", "Mon Delay Custom", "Nom :");
    alert->addButton ("Sauvegarder", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
    alert->addButton ("Annuler", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
    
    alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, alert] (int result) {
        if (result == 1)
        {
            juce::String name = alert->getTextEditorContents ("presetName");
            if (name.isNotEmpty())
            {
                audioProcessor.saveUserPreset (name);
                updatePresetsList();
            }
        }
        delete alert;
    }));
}

void DD2AudioProcessorEditor::deleteUserPreset()
{
    int currentProg = audioProcessor.getCurrentProgram();
    if (currentProg >= 5)
    {
        auto* alert = new juce::AlertWindow ("Supprimer Preset", 
                                             "Voulez-vous vraiment supprimer ce preset utilisateur ?", 
                                             juce::AlertWindow::WarningIcon);
        alert->addButton ("Oui", 1, juce::KeyPress (juce::KeyPress::returnKey, 0, 0));
        alert->addButton ("Non", 0, juce::KeyPress (juce::KeyPress::escapeKey, 0, 0));
        
        alert->enterModalState (true, juce::ModalCallbackFunction::create ([this, currentProg, alert] (int result) {
            if (result == 1)
            {
                audioProcessor.deleteUserPreset (currentProg);
                updatePresetsList();
            }
            delete alert;
        }));
    }
}
