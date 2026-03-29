This mod is possible because of the amazing work from the [refunct-tas](https://github.com/oberien/refunct-tas) team!

# Refunct for Archipelago

How does it work, what are items and locations?
- You will have to unlock every cluster of platforms (that normally rise by pressing a button).
- You also have to find your abilities to swim, ledge grab, wall kick and jump on jumppads!
- Location checks are making platforms grassy by jumping on 'em (200+ locations).
- Your goal is to collect enough grass and go to the final platform (adjustable via yaml, shown in-game).
- There are now minigames that give extra checks!

### Completely new to Archipelago?
Maybe [this](https://github.com/spinerak/refunct-tas-archipelago/blob/master/AP_HELP.md) helps.

## Here's how to play Refunct in Archipelago.
- You need to own the original game Refunct (via Steam for example).
- Download the `practice-windows.zip` or `practice-linux.zip` from the [release](https://github.com/spinerak/refunct-tas-archipelago/releases/latest). Unzip the contents into any folder.
- **\[Linux only]** Change the launch command for Refunct in Steam to `LD_PRELOAD="/path/to/practice-linux/librtil.so:libssl.so.3:libcrypto.so.3" %command%` (replace `/path/to/practice-linux` with the path to the unpacked `practice-linux` folder)
- Do the usual [Archipelago custom games things](https://archipelago.gg/tutorial/Archipelago/setup_en#playing-with-custom-worlds): get your yaml and apworld from the [release](https://github.com/spinerak/refunct-tas-archipelago/releases/latest), generate and host.
- Open Refunct.
- Once it's loaded, double-click `refunct-tas.exe` on Windows or `refunct-tas` on Linux (note: it may crash, just try again a couple of times, or check troubleshooting at the very bottom).
- After a short lag, you'll see "Press 'm' to open the menu" top-left. 
- There should be some more info on your screen that is good to read. Press 'm' to open the menu.
- Navigate to `Connect` and enter the login info. Example `archipelago.gg`  `38281`  `PlayerName` (you might have to put your name in notepad, copy it and then CTRL-V into the game if you have special characters)
- After closing the menu (escape), you should now see "Archipelago" and the request to start a new game. Open the Refunct menu (escape) and press New Game. Your game will start and you're good to go!

## Other stuff
- Softlocked? Press New Game, your progress is saved.
- Buttons and platforms underneath buttons are not checks in the main game. (Red) buttons don't do anything.
- Universal Tracker can help you to figure out which platforms are accessible. The mini-map should help!
- You should be able to quit a game and continue it later, just follow the steps again. Minigames do save checks but do not save progress.
- The final cluster in the game won't appear and will stay underwater in the main game (the final button is weird).

## Troubleshooting
### If the mod just crashes...
- Is Refunct open when you open the mod? We only support the latest version of Refunct via Steam.
- Please try it 4 or 5 times, sometimes it just crashes.
- On Linux, if the error says `thread 'main' panicked at 'called Result::unwrap()'...`, make sure that you configured `LD_PRELOAD` correctly.
- If you're still having problems, try updating your graphics drivers (yes this already helped many times).
- Feel free to ask in the discord (Archipelago -> future-game-design -> Refunct). We will probably ask for the error message you get when you run `debug.bat` and the log file `Users\user\AppData\Local\Temp\refunct-tas.log` (note that the log file may not exist).

## Building
If you want to build the project yourself, follow the steps from the original [refunct-tas](https://github.com/oberien/refunct-tas) project.
- For Windows: you might need to install "NASM - The Netwide Assembler" and add it to your PATH.

## Refunct TaS Tool
This tool is derived from https://github.com/oberien/refunct-tas.
There is a readme there too.
