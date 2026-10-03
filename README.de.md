# Thunder's Spielesammlung (Game Boy / Game Boy Pocket)

Eine kleine Spielesammlung für den originalen Game Boy und den Game Boy Pocket in 4 Graustufen, geschrieben in C mit GBDK-2020. Enthalten sind Breakout und Pong.

## Hauptmenü
- Großes „THUNDER'S“-Logo und ein Bild von Thunder neben dem Menü (`assets/avatar.png`, umgerechnet auf 56×56 Pixel in 4 Graustufen)
- BREAKOUT, PONG, LEVEL (Startlevel 1–9, gilt für beide Spiele), MUSIC (an/aus)
- Hintergrundmusik: „God Save the King“ (traditionell, gemeinfrei), zweistimmig (Melodie Kanal 1, Bass Kanal 2), läuft in Schleife
- Steuerung: ↑ ↓ Auswahl, ← → Wert ändern, A oder START startet das gewählte Spiel
- Nach Game Over geht es zurück ins Hauptmenü

## Spiel 1: Breakout

### Steuerung
- Steuerkreuz ← →: Paddle bewegen (mit Trägheit)
- A: Ball abschießen / mit Laser-Power-Up: Laser feuern (halten = Dauerfeuer)
- START: Pause, nächstes Level
- SELECT in der Pause: zurück ins Hauptmenü

### Physik
- Konstanter Ballbetrag (elastische Stöße), 8.8-Festkomma, 2 Teilschritte pro Frame
- X- und Y-Achse getrennt geprüft: exakte Spiegelung an Wänden und Steinen
- Paddle leicht gewölbt modelliert (Normale bis ca. 10° geneigt, Ablenkung bis 20°)
- Reibung: Paddle-Geschwindigkeit überträgt Impuls auf den Ball

### Sound
- Kanal 1: Stein (Tonhöhe je Reihe), Kanal 2: Paddle (tief) + leiser Wand-Klick
- Kanal 4: Ball verloren, Kanal 1 Sweep: Level geschafft

### Power-Ups
Fallen zufällig aus zerstörten Steinen (immer nur eines gleichzeitig).
Drop-Chance pro Stein: Level 1: 25 %, 2: 17 %, 3: 12,5 %, 4: 9 %, 5: 7 %, 6: 5,5 %, 7: 4,3 %, 8: 3,5 %, ab 9: 2,7 %.
- L (Laser, 40 %): 3 s lang mit A Steine abschießen
- W (Breit, 40 %): Paddle 10 s doppelt so breit
- Herz (Leben, 20 %): +1 Ball (max. 9)

Das Paddle blinkt in den letzten 1,5 s eines Power-Ups. Bei Ballverlust enden alle Effekte.
Nach „LEVEL CLEAR!“ geht es mit START ins nächste Level (Punkte und Bälle bleiben).

## Spiel 2: Pong
- Links Spieler, rechts CPU
- 3 eigene Tore → nächstes Level, die CPU wird stärker (schneller, reagiert früher, zielt genauer)
- 3 Gegentore → Game Over
- Physik wie bei Breakout, der Ball wird pro Ballwechsel mit jedem Schlägertreffer etwas schneller
- Sounds: eigener Schläger (tief), CPU-Schläger (höher), Bande (Klick), Tor (Rauschen bzw. Sweep)

### Steuerung
- Steuerkreuz ↑ ↓: Schläger bewegen
- A: Aufschlag zu Beginn eines Levels / nächstes Level
- START: Pause
- SELECT in der Pause: zurück ins Hauptmenü

## Bauen
GBDK-2020 4.3.0: `make GBDK=/pfad/zu/gbdk` → `thunder.gb`
Zum Neuerzeugen der Grafiken (`python3 gen_tiles.py`) wird Python 3 mit Pillow benötigt.

## Lizenz
Veröffentlicht unter der [MIT-Lizenz](LICENSE) © 2026 ThunderSooS.
Die Menümusik „God Save the King“ ist eine traditionelle, gemeinfreie Melodie.
