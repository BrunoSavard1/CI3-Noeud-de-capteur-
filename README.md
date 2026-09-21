# CI3-Noeud-de-capteur-

Projet éffectué par: Bruno Savard et William Champagne

Ce projet consiste à concevoir un PCB avec des capteurs environnementaux. Ces capteurs permettent de recueillir des informations telles que la luminosité, l’humidité, la qualité de l’air et la température ambiante d’une pièce. Le tout doit être intégré dans un boîtier compact afin de pouvoir facilement le déplacer et l’utiliser dans différents environnements.


## Requis électriques
- ESP32-C3-WROOM-02-N4 (MCU)
- ENS160-BGLM (capteur de qualité de l’air)
- SHT41AD1B-R2 (capteur de température et d’humidité)
- VEML7700 (capteur d’éclairement lumineux)
- Alimentation et programmation USB-C (5 V)
- DEL NeoPixel 3535 (diagnostic/état global)
- Bouton RESET (MCU)


## Requis optionnels
- Batterie LiPo (fusible ?)
- Diviseur de tension LiPo (3,7 V à 3,3 V)
- MOSFET PMOS (gestion de l’alimentation USB ou batterie)
- Connecteur B2B-PH-K-S (si LiPo)
- Circuit de recharge MCP73833-FCI/MF (si LiPo)
- PDT DFT

## Requis informatiques
- Interface des trois capteurs par bus I²C
- Programmation par USB-C
- Plateforme Arduino
- Plan de test du logiciel

## Requis mécaniques
- PCB de 50 × 50 mm
- Vis M3
- Boîtier
- Accès aux capteurs et au port USB-C
- Accès au bouton RESET à l’extérieur du boîtier
- Boîtier compact et robuste


### Matériel
**1.** Boitier

**2.** PCB    

### Logiciel
**1.** STM32CubeIDE, CubeMX et VsCode
    - Tkinter et Pillow doivent être installé  
    
    pip install Pillow
    pip install tk 

**1.** Altium -> PCB 
    - Librairie Celestial utilisée
    - Librairie étudiante utilisée

**3.** Fusion 360 -> Modèle 3D 

### Limitations
L'affichage est limité à une vitesse de rafraîchissement de 5 Hz.

La version actuelle est limitée à une utilisation stationnaire dû à l'écran HDMI nécessitant une station fixe.

## Structure

### 3DModels
Cette section regroupe les [modèles 3D](<3DModels/>) nécessaires pour la fabrication et la manufacturation du PCB et du support associé à ce même PCB.

**1.** Modèle 3D de la [carte du tableau de bord.](<3DModels/>)

![Carte PCB](3DModels/cartepcb.png "Carte PCB")

**2.** Modèle 3D du [support](<3DModels/>) pour la carte du tableau de bord et le raspberry pi. 

![Support](3DModels/support.png "Support")

### Code
Cette section contient les fichiers code du [STM32](<Code/STM32/>) et du [Raspberry Pi](<Code/STM32/>)

- #### Raspberry Pi 3b
**tk_main.py** code gérant l'affichage de l'interface de l'utilisateur et la communication UART. 

**icons** fichier contenant les icones nécessaire pour l'affichage de l'interface utilisateur.

- #### STM32

**stm32_main.c** code "pass through" de l'information uart, faisant le pont entre la carte de contrôle et le raspberry pi.

### Datasheets
Cette section contient les diverses [datasheets](<Datasheets/>) utilisées pour la conception du schéma électrique. 

### Guides
Cette section contient les [guides](<Guides/>) pour utilisateur et développeur.

### Journal

Cette section contient le [journal](<Journal/>) de bord, la distribution des tâches au travers de la journée, les difficultés rencontrées ainsi que les solutions trouvées à ces mêmes problèmes.

### PCB
Cette [section](<PCB/>) contient les divers [symboles, footprints et modèles](<PCB/Symbols, footprints and models>) nécessaire pour la conception de la carte PCB ainsi que le plan de test complété. 

#### Fichier des symboles, footprints et modèles
- Fichier contenant les éléments nécessaires pour la conception de la carte PCB.

#### Plan de test
- Plan de test complété.

### Schéma
La section ["Schematics"](<Schematics/>) du dépôt github contient les shémas suivant : 

**1.** Shéma bloc *Vielle version* 

**2.** Shéma électrique 

**3.** Ordinogramme | Pour le code du raspberry pi 

## Utilisation
**1.** Brancher et allumer les composantes

**2.** Lancer le logiciel *tk_main.exe* (À faire un executable du code python)

**3.** Utiliser la trottinette

### Code STM32 et Python
La section "Code" du dépôt github contient les codes suivant : 
- stm32_main.c -> Code main du microcontrolleur STM32.
- tk_main.py -> Code python pour l'écran.

### Pour un utilisateur
Voir [Guide d'utilisateur](<Guides/guide_user.md>)

### Pour un developpeur
Voir [Guide de développement](<Guides/guide_dev.md>)

### Modifications à faire
- Révision du PCB
     - Changement du régulateur
           - 12V -> 5V Switching Régulateur, 5V -> 3V3 Régulateur LDO. Cela nous permet d'intégrer l'alimentation par USB-C  
     - Changer les branchements du connecteur de debug
- Boitié étanche
- Remplacer le Raspberry PI avec un écran adapter pour une trottinette. Cette modification nous rapprocherait du projet initiale décris par le client.
      - Nécessite un changement du code, du schéma électrique (+ Routage) et du boitié. Cela est donc une modification plus importante à faire lorsque les autres modifications sont éffectuées.
