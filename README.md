# Thunder's Spielesammlung

Monochrome Game Boy / Game Boy Pocket homebrew, written in C with GBDK-2020. Current source version: 0.4.0.

[Deutsch](README.de.md) | [Downloads](https://github.com/ThunderSooS/thunders-spielesammlung/releases) | [Changes](CHANGELOG.md)

## Games

| Game | Goal | Controls |
|---|---|---|
| Breakout | Clear bricks; collect laser, wide-paddle and extra-life power-ups. | Left/right: paddle. A: launch/fire. |
| Pong | Score 3 goals to advance; concede 3 for game over. CPU difficulty increases. | Up/down: paddle. A: serve. |
| Bierkühler | Keep every can continuously at 5-8 °C for 180 gameplay updates. | Left/right: select. Each A press cools by 1 °C. |

START pauses/resumes. SELECT while paused returns to the menu. Bierkühler starts with two cans, adds one per level up to eight, and displays all cans with thermometers and progress bars. Below 0 °C or above 29 °C costs one life and restarts the level. No overall time limit; completed cans stay cold.

The menu provides start levels 1-9, music on/off, the picture, and a two-voice arrangement of "God Save the King". The third game is displayed as BIERKÜHLER.

## Build

Requires GBDK-2020 4.3.0, Git and Make:

```sh
make GBDK=/path/to/gbdk
```

Output: thunder.gb. Make applies bierkuehler.patch to build/main.c, leaving the tracked main.c unchanged. Do not compile main.c directly expecting all three games. GitHub Actions uses the same build and uploads thunder-rom.

Python 3 with Pillow is only needed for optional base/menu tile regeneration: python3 gen_tiles.py. Bierkühler graphics are separate in bierkuehler_graphics.inc.

## Releases and status

In Actions, run "Create release v0.4.0" on main. It builds that commit and creates a draft prerelease with thunder.gb and RELEASE_NOTES.md. Review the draft under Releases before publishing; existing releases are not overwritten.

The project owner reported successful emulator testing. Real-hardware testing has not been confirmed. This is a prototype targeting the original monochrome Game Boy and Game Boy Pocket.

## Files

main.c and pong.inc contain the base games. bierkuehler.inc contains the third game and buffered input; bierkuehler_graphics.inc its artwork and cached renderer; bierkuehler_menu_umlaut.inc the menu Ü. bierkuehler.patch integrates the menu during building. VERSION, CHANGELOG.md and RELEASE_NOTES.md track the release.

The images in docs/ are earlier prototype screenshots, not current Bierkühler images: [menu](docs/menu.png), [Breakout](docs/gameplay.png), [Pong](docs/pong.png).

## License

Project source: MIT, see LICENSE. This does not grant rights to third-party names, trademarks or material. Independent homebrew project.
