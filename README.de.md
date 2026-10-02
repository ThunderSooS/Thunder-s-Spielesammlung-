# Breakout für Game Boy Pocket – Version 2 (Power-Ups)

## Steuerung
- Steuerkreuz links/rechts: Paddle bewegen (mit Trägheit)
- A: Ball abschießen / mit Laser-Power-Up: Laser feuern (halten = Dauerfeuer)
- START: Spiel starten / Pause

## Physik
- Konstanter Ballbetrag (elastische Stöße), 8.8-Festkomma, 2 Teilschritte pro Frame
- X- und Y-Achse getrennt geprüft: exakte Spiegelung an Wänden und Steinen
- Paddle leicht gewölbt modelliert (Normale bis ca. 10° geneigt, Ablenkung bis 20°)
- Reibung: Paddle-Geschwindigkeit überträgt Impuls auf den Ball

## Sound
- Kanal 1: Stein (Tonhöhe je Reihe), Kanal 2: Paddle (tief) + leiser Wand-Klick
- Kanal 4: Ball verloren, Kanal 1 Sweep: gewonnen

## Bauen
GBDK-2020 4.3.0: `make GBDK=/pfad/zu/gbdk`

## Power-Ups
Fallen zufällig aus zerstörten Steinen (immer nur eines gleichzeitig).
Drop-Chance pro Stein: Level 1: 25 %, 2: 17 %, 3: 12,5 %, 4: 9 %, 5: 7 %, 6: 5,5 %, 7: 4,3 %, 8: 3,5 %, ab 9: 2,7 %.
- L (Laser, 40 %): 3 s lang mit A Steine abschießen
- W (Breit, 40 %): Paddle 10 s doppelt so breit
- Herz (Leben, 20 %): +1 Ball (max. 9)
Das Paddle blinkt in den letzten 1,5 s eines Power-Ups. Bei Ballverlust enden alle Effekte.
Nach „LEVEL CLEAR!“ geht es mit START ins nächste Level (Punkte und Bälle bleiben).
