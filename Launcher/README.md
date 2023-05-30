# Bonzomatic Network Launcher
![Screenshot](BonzoControl.png)

## What's this?
This is a tool to launch and manage several bonzomatic windows.
It was made to help with the Shader Royale competition by psenough.
The tool can:
- Launch several Bonzomatic Network windows, one for each distant coder
- Each windows can then be toggled visible
- Place the windows in a mosaic on your screen
- Display one of the windows in fullscreen while hiding all the others
- Launching a diaporama of all the windows
- Toggleing text editor on all the bonzo windows by pressing F11 in the control Windows
- Move the mosaic on a secondary screen while a coder is in full screen, so you can choose the next with a preview

By resizing the bonzomatic windows, this tool let them take more or less of the GPU power. With this, you can have dozen's of coder's shader running on one PC while still maintaining a good framerate and being able to put one in fullscreen when you want.
For streaming, the best is to dedicace one screen with a neutral background where all the bonzomatic windows will be placed and the whole screen will be reccorded

To use it, just unzip it in same folder as Bonzomatic Network.

## Keys
- M: Show mosaic mode
- D: Launch a diaporama
- O: Toggle displaying options
- R: Put a random coder in full screen
- A: Add a new coder
- [0-9]: coders 1-10 in full screen
- ctrl+[0-9]: coders 11-20 in full screen
- shift+[0-9]: coders 21-30 in full screen
- alt+[0-9]: coders 31-40 in full screen
- Left/right: show previous/next coder in full screen
- F11: toggle shader overlay on all bonzo windows

## Configuration
You can configure the launcher by tweaking the `launcher.json` that is next to the binary executable.

Example: (all fields are optional)
``` javascript
{
  "bonzo": {
    "commandline": "skipdialog networkMode=grabber",
    "delay_between_spawn": 1000,
    "exename": "Bonzomatic_W64_GLFW.exe"
  },
  "coders": [
    "Coder_01",
    "Coder_02",
    "Coder_03",
    "Coder_04"
  ],
  "diaporama": {
		"bpm": 60,
		"infiniteloop": false,
		"loops": 1 
	},
	"font": {
		"file": "ProFontWindows.ttf",
		"size": 16 
	},
	"fullscreen": {
		"forceratio": true,
		"monitor": 0,
		"sizepercent_x": 1,
		"sizepercent_y": 1,
		"startpercent_x": 0,
		"startpercent_y": 0,
		"wantedratio": 1.7777 
	},
	"list": {
		"codertogglemosaic": false,
		"hiddenatbottom": false 
	},
	"mosaic": {
		"MosaicFixed": false,
		"forceratio": true,
		"monitor": 0,
		"sizepercent_x": 0.9,
		"sizepercent_y": 0.9,
		"startpercent_x": 0.05,
		"startpercent_y": 0.05,
		"wantedratio": 1.7777 
	},
  "network": {
    "usenetwork": false,
    "autoaddcoder": true,
    "autolaunchbonzo": true,
    "autolaunchbonzonetwork": false,
    "serverURL": "ws:\/\/drone.alkama.com:9000\/roomtest\/"
  },
	"secondary": {
		"forceratio": true,
		"monitor": 1,
		"sizepercent_x": 0.4,
		"sizepercent_y": 0.4,
		"startpercent_x": 0.4,
		"startpercent_y": 0.4,
		"use": true,
		"wantedratio": 1.7777 
	},
  "theme": {
    "background": "202020",
    "text": "FFFFFF",
    "button": "666666",
    "buttonHover": "999999",
    "buttonPress": "FFFFFF",
    "buttonUncheck": "CC3333",
    "buttonUncheckHover": "FF8080",
    "scrollbar": "404040"
  }
} 
```

This tool has been written by NuSan and is public domain.
