# Thunder's Spielesammlung

Homebrew für den monochromen Game Boy / Game Boy Pocket, geschrieben in C mit GBDK-2020. Aktuelle Quellcode-Version: 1.0.0.

[English](README.md) | [Downloads](https://github.com/ThunderSooS/thunders-spielesammlung/releases) | [Änderungen](CHANGELOG.md)

## Spiele

| Spiel | Ziel | Steuerung |
|---|---|---|
| Breakout | Steine zerstören; Laser, breiten Schläger und zusätzliche Leben einsammeln. | Links/rechts: Schläger. A: Ball starten/Laser. |
| Pong | 3 eigene Tore: nächstes Level. 3 Gegentore: Game Over. Die CPU wird stärker. | Hoch/runter: Schläger. A: Aufschlag. |
| Bierkühler | Jede Dose durchgehend für 180 Spielaktualisierungen zwischen 5 und 8 °C halten. | Links/rechts: Dose wählen. Jeder A-Tastendruck kühlt um 1 °C. |

START pausiert/setzt fort. SELECT in der Pause führt ins Menü. Bierkühler beginnt mit zwei Dosen und ergänzt pro Level eine, bis maximal acht. Alle sind mit Thermometern und Fortschrittsbalken sichtbar. Unter 0 °C oder über 29 °C geht ein Leben verloren und das Level startet neu. Kein allgemeines Zeitlimit; fertige Dosen bleiben kalt.

Das Menü bietet Startlevel 1-9, Musik an/aus, das Bild und eine zweistimmige Fassung von „God Save the King“. Der dritte Spieleintrag heißt BIERKÜHLER mit echtem Ü.

## Grafik

Bierkühler zeigt detaillierte Metallränder, Dosenlaschen, seitliche Schattierung und Kondenswassertropfen. Die einzeiligen Diebels-Etiketten bleiben erhalten. Die Thermometer haben eine schattierte Kugel und deutliche Zielbereich-Markierungen. Neben OK erscheint bei fertigen Dosen ein kleiner Glanzstern. Es gibt keine dekorativen Rahmen oben oder unten.

## Teststand

Erfolgreich im Emulator und auf einem echten Game Boy Pocket mit Flashcard getestet. Der Projektinhaber hat bestätigt, dass der aktuelle Spielstand für v1.0.0 auf dem Pocket gut läuft. Tests auf anderen echten Game-Boy-Modellen werden nicht behauptet.

## Bauen

Benötigt: GBDK-2020 4.3.0, Git und Make:

```sh
make GBDK=/pfad/zu/gbdk
```

Ergebnis: thunder.gb. Make wendet bierkuehler.patch auf build/main.c an; die gespeicherte main.c bleibt unverändert. Die main.c allein nicht direkt kompilieren, wenn alle drei Spiele enthalten sein sollen. GitHub Actions verwendet denselben Build und stellt thunder-rom bereit.

Python 3 mit Pillow wird nur zum optionalen Neuerzeugen der Basis-/Menügrafiken benötigt: python3 gen_tiles.py. Die Bierkühler-Grafiken stehen separat in bierkuehler_graphics.inc.

## Release vorbereiten

Nach einem erfolgreichen Build thunder-rom herunterladen, thunder.gb entpacken und diese ROM testen. Ein neues Release mit dem Tag v1.0.0 auf dem abschließend geprüften Commit von main anlegen. RELEASE_NOTES.md als Beschreibung verwenden, thunder.gb anhängen, die normale Release-Kennzeichnung wählen und erst nach der Abschlussprüfung veröffentlichen. v0.4.0 bleibt unverändert.

Der vorhandene Workflow „Create release v0.4.0“ ist an die ältere Version gebunden und darf nicht zum Erstellen von v1.0.0 verwendet werden. Diese Dokumentationsänderungen veröffentlichen selbst kein Release.

## Dateien

main.c und pong.inc enthalten die Basisspiele. bierkuehler.inc enthält das dritte Spiel mit Eingabepuffer; bierkuehler_graphics.inc die Grafiken und optimierte Darstellung; bierkuehler_menu_umlaut.inc das Menü-Ü. bierkuehler.patch bindet das Menü beim Bauen ein. VERSION, CHANGELOG.md und RELEASE_NOTES.md dokumentieren die Version.

Die Bilder in docs/ zeigen ältere Prototypen, nicht den aktuellen Bierkühler: [Menü](docs/menu.png), [Breakout](docs/gameplay.png), [Pong](docs/pong.png).

## Lizenz

Projektquellcode: MIT, siehe LICENSE. Keine Rechte an fremden Namen, Marken oder Fremdmaterial werden damit eingeräumt. Unabhängiges Homebrew-Projekt.
