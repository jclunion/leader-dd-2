/*
  ==============================================================================

    PluginProcessor.cpp
    Created: 30 May 2026
    Author: LUNION jean-Claude
    Description: Implémentation du processeur audio principal pour le Boss DD-2.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// --- Constructeur ---
DD2AudioProcessor::DD2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
       apvts (*this, nullptr, "DD2Parameters", createParameterLayout())
{
    // Liaison des pointeurs atomiques de paramètres
    eLevelParam   = apvts.getRawParameterValue ("elevel");
    feedbackParam = apvts.getRawParameterValue ("feedback");
    dTimeParam    = apvts.getRawParameterValue ("dtime");
    modeParam     = apvts.getRawParameterValue ("mode");
    holdParam     = apvts.getRawParameterValue ("hold");

    // Initialisation des presets d'usine (Factory Presets)
    factoryPresets = {
        { "Classic Slapback", 0.35f, 0.15f, 0.33f, 1 },      // ~100ms slapback, low feedback (Mode 2)
        { "Warm Analog Echo", 0.45f, 0.45f, 0.25f, 2 },      // ~350ms, low-pass damping (Mode 3)
        { "Dreamy Oscillation", 0.50f, 1.05f, 0.40f, 2 },    // ~440ms, self-oscillation feedback (Mode 3)
        { "Retro 12-Bit Grit", 0.40f, 0.70f, 0.16f, 2 },     // ~300ms, prominent lo-fi digital noise/grit (Mode 3)
        { "Infinite Hold Loop", 0.60f, 0.50f, 0.50f, 3 }     // HOLD Mode pre-setup (Mode 4)
    };
    currentProgramIndex = 0;

    // Charger les presets utilisateur au démarrage
    scanUserPresets();
}

DD2AudioProcessor::~DD2AudioProcessor()
{
}

// --- Layout des Paramètres ---
juce::AudioProcessorValueTreeState::ParameterLayout DD2AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // E.Level (Effect Level) : 0.0 à 1.0 (Niveau du signal Wet)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("elevel", 1),
        "E.Level",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));

    // F.Back (Feedback) : 0.0 à 1.15 (Permet l'auto-oscillation au-delà de 1.0)
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("feedback", 1),
        "F.Back",
        juce::NormalisableRange<float> (0.0f, 1.15f, 0.01f),
        0.5f));

    // D.Time (Delay Time) : Multiplicateur continu 0.0 à 1.0
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID ("dtime", 1),
        "D.Time",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.5f));

    // Mode Switch : 4 Positions (Short, Med, Long, HOLD)
    juce::StringArray modeChoices { "12.5-50ms", "50-200ms", "200-800ms", "HOLD" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID ("mode", 1),
        "Mode",
        modeChoices,
        2)); // Par défaut : Long (200-800ms)

    // Commutateur de pied HOLD (Active le bouclage infini en mode 4)
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID ("hold", 1),
        "Hold",
        false));

    return { params.begin(), params.end() };
}

// --- Cycle de vie audio ---
const juce::String DD2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DD2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DD2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DD2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DD2AudioProcessor::getTailLengthSeconds() const
{
    // Tail length depend sur le feedback
    return 5.0; 
}

int DD2AudioProcessor::getNumPrograms()
{
    return 5 + static_cast<int> (userPresets.size());
}

int DD2AudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void DD2AudioProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < getNumPrograms())
    {
        currentProgramIndex = index;
        
        if (index < 5) // Preset d'usine
        {
            auto& preset = factoryPresets[static_cast<size_t> (index)];

            if (auto* p = apvts.getParameter ("elevel"))   p->setValueNotifyingHost (preset.elevel);
            if (auto* p = apvts.getParameter ("feedback")) p->setValueNotifyingHost (preset.feedback / 1.15f);
            if (auto* p = apvts.getParameter ("dtime"))    p->setValueNotifyingHost (preset.dtime);
            if (auto* p = apvts.getParameter ("mode"))     p->setValueNotifyingHost (static_cast<float> (preset.mode) / 3.0f);
        }
        else // Preset Utilisateur
        {
            int userIndex = index - 5;
            if (userIndex >= 0 && userIndex < static_cast<int> (userPresets.size()))
            {
                auto& preset = userPresets[static_cast<size_t> (userIndex)];
                std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (preset.file));
                if (xml.get() != nullptr)
                {
                    apvts.replaceState (juce::ValueTree::fromXml (*xml));
                }
            }
        }
    }
}

const juce::String DD2AudioProcessor::getProgramName (int index)
{
    if (index >= 0 && index < 5)
    {
        return factoryPresets[static_cast<size_t> (index)].name;
    }
    else if (index >= 5 && index < getNumPrograms())
    {
        int userIndex = index - 5;
        if (userIndex >= 0 && userIndex < static_cast<int> (userPresets.size()))
            return userPresets[static_cast<size_t> (userIndex)].name;
    }
    return {};
}

void DD2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index >= 0 && index < 5)
    {
        factoryPresets[static_cast<size_t> (index)].name = newName;
    }
    else if (index >= 5 && index < getNumPrograms())
    {
        int userIndex = index - 5;
        if (userIndex >= 0 && userIndex < static_cast<int> (userPresets.size()))
            userPresets[static_cast<size_t> (userIndex)].name = newName;
    }
}

// --- Implémentation de la gestion des presets utilisateur sur disque ---
void DD2AudioProcessor::scanUserPresets()
{
    userPresets.clear();
    
    auto folder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("DD2Emulation")
                    .getChildFile ("UserPresets");
                    
    if (!folder.exists())
    {
        folder.createDirectory();
    }
    
    if (folder.isDirectory())
    {
        juce::Array<juce::File> files;
        folder.findChildFiles (files, juce::File::findFiles, false, "*.dd2preset");
        
        for (auto& file : files)
        {
            std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
            if (xml.get() != nullptr)
            {
                juce::String name = xml->getStringAttribute ("presetName", file.getFileNameWithoutExtension());
                userPresets.push_back ({ name, file });
            }
        }
    }
}

void DD2AudioProcessor::saveUserPreset (const juce::String& presetName)
{
    auto folder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("DD2Emulation")
                    .getChildFile ("UserPresets");
                    
    if (!folder.exists())
    {
        folder.createDirectory();
    }
    
    juce::String safeName = juce::File::createLegalFileName (presetName);
    auto file = folder.getChildFile (safeName + ".dd2preset");
    
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    xml->setAttribute ("presetName", presetName);
    
    xml->writeTo (file);
    
    scanUserPresets();
    
    // Active le preset nouvellement créé
    currentProgramIndex = 5 + static_cast<int> (userPresets.size()) - 1;
}

void DD2AudioProcessor::deleteUserPreset (int index)
{
    if (index >= 5 && index < getNumPrograms())
    {
        int userIndex = index - 5;
        if (userIndex >= 0 && userIndex < static_cast<int> (userPresets.size()))
        {
            userPresets[static_cast<size_t> (userIndex)].file.deleteFile();
            scanUserPresets();
            setCurrentProgram (0); // Revenir au premier preset d'usine
        }
    }
}

void DD2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Préparation du moteur DSP
    dspEngine.prepareToPlay (sampleRate, samplesPerBlock);
}

void DD2AudioProcessor::releaseResources()
{
    dspEngine.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DD2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Le plugin supporte uniquement les configurations d'E/S stéréo-stéréo ou mono-mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

// --- Traitement Audio (Real-time safety) ---
void DD2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    
    // Protection contre les clics et débordements si aucun échantillon n'est présent
    if (buffer.getNumSamples() == 0)
        return;

    juce::ScopedNoDenormals noDenormals;

    // Lecture atomique ultra-rapide des valeurs des paramètres
    float currentELevel   = eLevelParam   ->load();
    float currentFeedback = feedbackParam ->load();
    float currentDTime    = dTimeParam    ->load();
    int   currentMode     = static_cast<int> (modeParam->load());
    bool  currentHold     = holdParam     ->load() > 0.5f;

    // Exécution du moteur DSP principal Boss DD-2
    dspEngine.process (buffer, currentELevel, currentFeedback, currentDTime, currentMode, currentHold);
}

// --- Interface graphique (Editor) ---
bool DD2AudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* DD2AudioProcessor::createEditor()
{
    return new DD2AudioProcessorEditor (*this);
}

// --- Persistance de l'état (Saving / Loading Presets) ---
void DD2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Copie de l'état actuel de l'APVTS au format XML pour sauvegarde
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DD2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Chargement de l'état à partir des données binaires
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
    }
}

// --- Point d'entrée de création du plugin requis par JUCE ---
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DD2AudioProcessor();
}
