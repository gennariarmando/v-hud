<p align="center"><img src="https://i.imgur.com/Gp5g7l2.png" width="512" height="128"></p>

[<p align="center"><img src="https://discordapp.com/api/guilds/941825090717356053/widget.png?style=banner2" alt="Tw@"/>](https://discord.gg/xAvbhp63sg)

# V Hud:
A work-in-progress user interface overhaul, for Grand Theft Auto: San Andreas, based on Grand Theft Auto: V.\
Project has been made in order to replicate accurately most of the UI elements present in GTA:V.

## Screenshots:
<p align="center">
<img src="https://i.imgur.com/G9fhw0E.png" width="320" height="180"><img src="https://i.imgur.com/ISOmF3P.png" width="320" height="180">
<img src="https://i.imgur.com/TV5qmcO.png" width="320" height="180"><img src="https://i.imgur.com/fKxZv8I.png" width="320" height="180">
<img src="https://i.imgur.com/7xrzjgA.png" width="320" height="180"><img src="https://i.imgur.com/JZY26Cg.png" width="320" height="180">
<img src="https://i.imgur.com/gGr1yzg.png" width="320" height="180"><img src="https://i.imgur.com/wAjQ0tO.png" width="320" height="180">
<img src="https://i.imgur.com/OOgpTc7.png" width="320" height="180"><img src="https://i.imgur.com/GdWLngG.png" width="320" height="180">
<img src="https://i.imgur.com/cjXjlXq.png" width="320" height="180"><img src="https://i.imgur.com/i21vjnY.png" width="320" height="180">
</p>

## System Requirements:
- Requires 2GB or more of RAM
- DirectX 9 compatible GPU with Pixel Shader 3.0
- Minimum screen resolution: 800x600

## Compiling:
Requirements:
 - Visual Studio 2019
 - [Plugin SDK](https://github.com/DK22Pac/plugin-sdk)
 
#### Submodules needs to be updated in order to build this project, run the following command after cloning the repository:
 ```
git submodule update --init --recursive
```

## Download
Download the latest archive from the [releases](https://github.com/gennariarmando/v-hud/releases) page.

# Installation:
#### Installing an ASI Loader:
An ASI Loader is required in order to inject the plugin into the game, if you already have one skip to the next step.\
Recommended: [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader)

Place the DLL file (renamed into "vorbisFile.dll") into your GTA directory.

#### Installing V Hud:
Archive content: 
- VHud.asi
- VHud folder
- bass.dll

Copy the content of the archive into your GTA directory.\
Attention: The "VHud" folder MUST be placed in the same directory where the asi file is.

## Compatibility:
Currently compatible only with GTA: San Andreas 1.0 US.

## Incompatibility
This plugin might be incompatible with a heavily modded game.\
Plugins that modify and change:
 - User interface related stuff
 - Loading screens (new loading screens, jump directly to game modes, etc...)
 - Game window attributes (Windowed Mode, etc...)
 - Graphical effects through vertex shaders (ENB Series, RenderHook, etc...)
 
may be incompatible with this plugin and will make the game run improperly, or in most cases not run at all.

#### Please make tests on your own on a unmodified version of the game, or at least on a lightly modded game, before opening any [issue](https://github.com/gennariarmando/v-hud/issues)

In the future some compatibility issues might be solved.

## Contributing:
You can contribute in multiple ways, share code, knowledge, ideas, bug reports.

## Links:
- [plugin-sdk](https://github.com/DK22Pac/plugin-sdk)
- [bass](https://www.un4seen.com/)
- [pugixml](https://github.com/zeux/pugixml)
- [GInputSA](https://gtaforums.com/topic/562765-ginput/)
- [ModUtils](https://github.com/CookiePLMonster/ModUtils)
- [GTA5 HUD by DK22Pac](https://gtaforums.com/topic/652697-gta5-hud-by-dk22pac/)
