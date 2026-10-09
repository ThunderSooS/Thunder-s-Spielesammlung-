# Thunder's Spielesammlung

Monochrome Game Boy / Game Boy Pocket homebrew, written in C with GBDK-2020. Published version: 1.0.0.

[Deutsch](README.de.md) | [Release v1.0.0](https://github.com/ThunderSooS/thunders-spielesammlung/releases/tag/v1.0.0) | [Changes](CHANGELOG.md)

## Download and play

1. Download [thunder.gb](https://github.com/ThunderSooS/thunders-spielesammlung/releases/download/v1.0.0/thunder.gb), the playable ROM attached to release v1.0.0.
2. Open thunder.gb in a Game Boy emulator, or use your flash cartridge setup on a Game Boy Pocket.
3. Select a game in the main menu and start playing.

The source-code ZIP and TAR archives are for development; they are not the playable ROM. No compilation is needed to use the downloaded thunder.gb.

## Games

| Game | Goal | Controls |
|---|---|---|
| Breakout | Clear bricks; collect laser, wide-paddle and extra-life power-ups. | Left/right: paddle. A: launch/fire. |
| Pong | Score 3 goals to advance; concede 3 for game over. CPU difficulty increases. | Up/down: paddle. A: serve. |
| Bierkühler | Keep every can continuously at 5-8 °C for 180 gameplay updates. | Left/right: select. Each A press cools by 1 °C. |

START pauses/resumes. SELECT while paused returns to the menu. Bierkühler starts with two cans, adds one per level up to eight, and displays all cans with thermometers and progress bars. Below 0 °C or above 29 °C costs one life and restarts the level. No overall time limit; completed cans stay cold.

The menu provides start levels 1-9, music on/off, the picture, and a two-voice arrangement of "God Save the King". The third game is displayed as BIERKÜHLER.

## Graphics

Bierkühler has detailed metal rims, pull tabs, cylindrical shading and condensation droplets. The single-line Diebels labels are retained. Thermometers have a shaded bulb and clear target-range brackets. A small glint appears beside OK for completed cans. The screen has no decorative top or bottom borders.

## Testing

Tested successfully in a Game Boy emulator and on a real Game Boy Pocket using a flash cartridge. The project owner has confirmed that the v1.0.0 game build runs well on the Pocket. Testing on other physical Game Boy models is not claimed.

## Building and developer notes

Requires GBDK-2020 4.3.0, Git and Make:

```sh
make GBDK=/path/to/gbdk
```

Output: thunder.gb. Make applies bierkuehler.patch to build/main.c, leaving the tracked main.c unchanged. Do not compile main.c directly expecting all three games. GitHub Actions uses the same build and uploads thunder-rom.

Python 3 with Pillow is only needed for optional base/menu tile regeneration: python3 gen_tiles.py. Bierkühler graphics are separate in bierkuehler_graphics.inc.

The existing "Create release v0.4.0" workflow is specific to the older version and is not the workflow for v1.0.0. Documentation updates on main do not replace published ROMs or change existing release tags.

## Files

main.c and pong.inc contain the base games. bierkuehler.inc contains the third game and buffered input; bierkuehler_graphics.inc its artwork and cached renderer; bierkuehler_menu_umlaut.inc the menu Ü. bierkuehler.patch integrates the menu during building. VERSION, CHANGELOG.md and RELEASE_NOTES.md track the release.

## Earlier screenshots

The images in docs/ are earlier prototype screenshots, not current Bierkühler images: [menu](docs/menu.png), [Breakout](docs/gameplay.png), [Pong](docs/pong.png).

## License

Project source: MIT, see LICENSE. This does not grant rights to third-party names, trademarks or material. Independent homebrew project.
