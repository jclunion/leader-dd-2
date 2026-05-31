# LEADER DD-2 - Émulation de Délai Numérique Vintage 12 bits
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Bienvenue dans le projet **LEADER DD-2**, un plugin audio VST3/AU de qualité professionnelle, prêt pour la production, développé en C++ avec le framework JUCE 7+. Ce plugin est une émulation physique et mathématique de haute fidélité de la légendaire pédale de retard numérique compacte **Boss DD-2** sortie en 1983 — la toute première pédale de delay numérique au format compact au monde.

Pour des raisons de respect des marques déposées et de droits d'auteur tout en préservant l'esthétique mythique des années 80, le plugin a été rebaptisé sous le nom de marque **LEADER**, un hommage authentique à l'ingénierie et au design rétro des appareils de mesure et synthétiseurs japonais de l'âge d'or de l'électronique.

![Interface LEADER DD-2](https://i.imgur.com/EaLWhSB.png)

---

## I. SPÉCIFICATIONS CLÉS ET AMÉLIORATIONS DSP

Contrairement aux délais numériques génériques modernes qui répètent le son de façon chirurgicale et transparente, le **LEADER DD-2** modélise fidèlement les limites technologiques du convertisseur numérique et la chaleur des circuits de support analogiques d'origine :

### 1. Cœur Numérique Vintage 12 bits / 32 kHz
*   **Émulation Sample-and-Hold :** Le signal entrant dans la ligne de retard est décimé via un bloqueur d'ordre zéro (*Zero-Order Hold*) à une fréquence d'échantillonnage stricte de **32 000 Hz**, quelle que soit la fréquence de travail de votre session DAW (supporte jusqu'à 192 kHz).
*   **Quantification 12 bits :** L'amplitude du signal est strictement quantifiée sur une plage dynamique de 12 bits ($2^{12} = 4096$ valeurs discrètes), convertissant le signal dans la plage $[-2048, 2047]$ avant de le ramener à l'échelle flottante.

### 2. Émulation Premium du Bruit de LSB et du MSB Glitch
*   **Dither Thermique du LSB (-65 dB) :** Modélise le bruit de fond analogique d'entrée des premiers convertisseurs. En injectant un très faible bruit blanc thermique avant le quantificateur, nous forçons le bit de poids faible (LSB) à basculer de façon aléatoire. Cela génère un souffle numérique vintage très organique et caractéristique sur la fin des échos.
*   **Distorsion de Commutation MSB (MSB Glitch / Crossover Distortion) :** Les puces de conversion 12 bits de l'époque (comme la célèbre PCM53) présentaient d'importantes non-linéarités et des erreurs de commutation lors du passage par zéro, au moment où le bit de poids fort (MSB) basculait. Nous avons modélisé cette erreur avec une distorsion sinusoïdale localisée sur les signaux inférieurs à $0,07$ d'amplitude. Lorsque vos échos s'éteignent doucement, ils se dissolvent dans une **texture numérique granuleuse, râpeuse et métallique d'un charme absolu**, typique du son lo-fi des années 80 !

### 3. Compandeur NE570 en Boucle Fermée (Feedback)
*   **Compresseur Feedback Réel (2:1) :** Les pédales de cette époque utilisaient le circuit NE570 pour réduire le souffle. Sur le matériel d'origine, le détecteur d'enveloppe de gain est branché sur la **sortie** du compresseur (topologie feedback). Nous avons programmé cette boucle fermée échantillon par échantillon avec des constantes temporelles de $4\text{ ms}$ à l'attaque et $10\text{ ms}$ au relâchement. Cela restitue de **magnifiques sursauts transitoires (transient overshoot)** redonnant tout le punch et le claquant initial à vos attaques de médiator.
*   **Expanseur Feedforward (1:2) :** À la sortie de la ligne de retard, le signal Wet est expansé dynamiquement à l'aide d'un suiveur d'enveloppe branché en action directe, complétant ainsi le cycle de réduction de bruit dynamique.

### 4. Filtres SVF de Simper en Cascade (Pente Raide)
*   **Anti-repliement pré-A/D (7 kHz) :** Un filtre actif passe-bas raide de 4ème ordre (constitué de deux filtres à variables d'état (SVF) de Simper de 2nd ordre branchés en série) élimine les hautes fréquences avant la décimation.
*   **Reconstruction post-D/A (7 kHz) :** Un autre filtre en cascade SVF de 4e ordre calé à 7 kHz lisse les paliers de quantification en escalier des 12 bits, éliminant toute agressivité numérique tout en donnant aux répétitions leur rondeur acoustique.
*   **Amortissement de Boucle (Feedback Damping à 3 kHz) :** Un filtre SVF de 2nd ordre passe-bas atténue en douceur les hauts-médiums à chaque répétition, rendant chaque écho successivement plus sombre et chaud.

### 5. Auto-oscillation Stabilisée
*   Le potentiomètre **F.Back** s'étend de $0,0$ à $1,15$, permettant d'entrer en auto-oscillation complète.
*   Un circuit d'écrêtage doux non linéaire basé sur la fonction tangente hyperbolique ($\tanh$) a été intégré dans la boucle fermée pour stabiliser les signaux en sur-réaction et obtenir des saturations harmoniques chaleureuses.

---

## II. CONTRÔLES ET MODES MATÉRIELS

*   **E.Level (Effect Level) :** Dosage Wet/Dry (le signal direct Dry reste à gain unitaire, tandis que E.Level injecte de $0\%$ à $100\%$ du signal d'écho).
*   **F.Back (Feedback) :** Contrôle le nombre de répétitions, du simple écho slapback jusqu'à l'auto-oscillation grasse.
*   **D.Time (Delay Time) :** Ajuste continuellement le temps de retard dans la plage du mode actif.
*   **Sélecteur de Mode (Rotatif 4 Positions) :**
    *   **Mode 1 :** 12,5 ms à 50 ms (Délai court / Doublage / Chorus)
    *   **Mode 2 :** 50 ms à 200 ms (Écho Slapback moyen)
    *   **Mode 3 :** 200 ms à 800 ms (Délai numérique long)
    *   **Mode 4 (HOLD) :** Gèle la mémoire numérique au moment où le footswitch est enfoncé, créant une boucle parfaite.
*   **Pédale HOLD (Footswitch) :** Déclenche la boucle infinie dans la RAM numérique en Mode 4, sans aucune dégradation de filtre lors de la boucle fermée.

---

## III. GESTIONNAIRE DE PRESETS UTILISATEUR AUTONOME (SUR L'INTERFACE)

Pour s'affranchir des limitations de gestion des presets VST3 de certains DAWs (comme Ableton Live), nous avons conçu un **gestionnaire de presets 100% autonome et persistant** intégré directement en haut de l'interface graphique :
*   **Sélecteur de Presets (En haut à gauche) :** Un menu déroulant rétro-industriel (`juce::ComboBox`) listant de façon unifiée les presets d'usine et vos presets utilisateur.
*   **Synchronisation Bidirectionnelle :** Communique parfaitement avec l'hôte DAW et met à jour instantanément les curseurs physiques de la pédale au changement de preset.
*   **Bouton SAVE (En haut à droite) :** Ouvre une boîte de dialogue interactive pour nommer et enregistrer votre preset sous forme de fichier XML persistant sur votre disque (`C:\Users\<NomUtilisateur>\AppData\Roaming\DD2Emulation\UserPresets\`).
*   **Bouton DEL (En haut à droite) :** Supprime physiquement le fichier de preset du disque (bouton automatiquement grisé/désactivé pour les presets d'usine afin d'éviter toute fausse manipulation) et recharge le preset par défaut.

### Les 5 Presets d'Usine Inclus :
1.  **`Classic Slapback`** : Délai court d'environ 100 ms avec peu de retour, idéal pour doubler une voix ou du rockabilly.
2.  **`Warm Analog Echo`** : Délai chaleureux de 350 ms avec fort amortissement des aigus, émulant les circuits analogiques BBD.
3.  **`Dreamy Oscillation`** : Délai planant de 440 ms avec retour poussé à 1,05, générant des vagues d'auto-oscillation ambiantes stabilisées.
4.  **`Retro 12-Bit Grit`** : Délai lo-fi de 300 ms mettant en valeur le bruit de LSB et le crossover glitch métallique.
5.  **`Infinite Hold Loop`** : Pré-configuration optimisée pour capturer des boucles à la volée avec le commutateur HOLD.

---

## IV. COMPILATION & INTÉGRATION

Le projet est entièrement structuré et prêt à compiler. Les fichiers sources sont situés dans le dossier `Source/` :
*   `Source/DD2Engine.h` & `DD2Engine.cpp` (Le Cœur DSP)
*   `Source/PluginProcessor.h` & `Source/PluginProcessor.cpp` (Le Processeur principal)
*   `Source/PluginEditor.h` & `Source/PluginEditor.cpp` (L'Éditeur graphique de la pédale)

### Option A : Avec le Projucer
1.  Ouvrez le logiciel Projucer et cliquez sur **Open Existing Project...**
2.  Sélectionnez le fichier `NewProject/NewProject.jucer`.
3.  Vérifiez que le **Project Type** est bien réglé sur **Audio Plug-In** (dans Project Settings).
4.  Retirez le fichier `Main.cpp` de l'explorateur de fichiers sous le dossier `Source/` s'il est présent (un plugin utilise l'entrée du processeur).
5.  Sauvegardez le projet (**Ctrl + S**), ce qui va régénérer la solution Visual Studio ou Xcode.
6.  Ouvrez la solution dans votre IDE de compilation, sélectionnez votre cible (VST3, AU, ou Standalone) et lancez **Recompiler**.

### Option B : Avec CMake
Ajoutez simplement les sources dans votre `CMakeLists.txt` :
```cmake
juce_add_plugin(DD2Emulation
    COMPANY_NAME "Antigravity"
    PLUGIN_MANUFACTURER_CODE "Antg"
    PLUGIN_CODE "Dd2e"
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "LEADER DD-2"
)

target_sources(DD2Emulation PRIVATE
    Source/DD2Engine.h
    Source/DD2Engine.cpp
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)
```

Profitez bien du grain lo-fi légendaire et de la chaleur dynamique du **LEADER DD-2** !
