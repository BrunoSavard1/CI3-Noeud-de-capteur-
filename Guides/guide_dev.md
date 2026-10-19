# Guide de développeur


## Avertissement

1. Utiliser les images de références de silkscreens pour éffectuer un assemblage du pcb acceptable.
2. Pour faire rentrer le connecteur de batterie (qui doit pointer vers le bas), il faut enlever le casing du connecteur.
3. Vérifier la polarité du connecteur de batterie. Un short circuit est si vite arrivé.
4. Vérifier l'orientation de la TVS, cela peut causer des problèmes de communication avec l'ordinateur.

## PlatformIO

Pour une compréhension et utilisation adéquate, suiver les informations fournis par PlatformIO: https://platformio.org/

## Programmation

1. Maintenir RESET 
2. Maintenir BOOT
3. Relacher RESET
4. Relacher BOOT

Cela enclenche le mode programmationz