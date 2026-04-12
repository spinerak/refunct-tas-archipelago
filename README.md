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
- You need to own the original game Refunct (via Steam).
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
- Universal Tracker can help you to figure out which platforms are accessible. The mini-map should help!
- You should be able to quit a game and continue it later, just follow the steps again. Minigames do save checks but do not save progress.

## Troubleshooting
All of these troubleshooting steps are for when the mod crashes on launch.
### The basics
- Is Refunct open when you open the mod? We only support the latest version of Refunct via Steam (build id 5753767).
- Please try it 2 or 3 times, sometimes it just crashes.
### Advanced
- There are two ways to learn more about the error.
  - Run `debug.bat` (after opening Refunct) and it'll print an error.
  - Find the log file at `Users\user\AppData\Local\Temp\refunct-tas.log` (note that the log file may not exist).
- **On linux, `debug.bat` says `thread 'main' panicked at 'called Result::unwrap()'...`]**
  - Make sure that you configured `LD_PRELOAD` correctly.
- **`debug.bat` says `"failed to fill whole buffer"`**
  - Do you have Spybot system tray? That program might interfere with Refunct.
- **Log file ends with `Got code`**
  - You might have older graphics drivers.
  - Right-click the game on steam and add this as launch option: `cmd /C "set WGPU_BACKEND=gles&& %command%"`
  - Or you could try to update your graphics driver (this helped many times).
- **`debug.bat` says `ParseIntError { kind: InvalidDigit }`**
  - Refunct probably has multiple processes open (some kind of overlay)?
  - Please run Refunct, then open a cmd, and run `powershell -NoProfile -Command "Get-Process -Name 'Refunct-Win32-Shipping'"`. If it shows two or more lines, then this does seem to be the case.
  - Try to kill one of the processes (try the one with the lowest handles/cpu first) using `taskkill /Pid 8132` where `8132` is the ID. If Refunct doesn't crash, you got the correct one
  - Now try to run the mod again.
     
### Stay calm and don't panic
- Feel free to ask in the discord (Archipelago -> future-game-design -> Refunct). We will probably ask for your operating system and the error messages you got, and how/when it crashes. 
## Building
If you want to build the project yourself, follow the steps from the original [refunct-tas](https://github.com/oberien/refunct-tas) project.
- For Windows: you might need to install "NASM - The Netwide Assembler" and add it to your PATH.

## Refunct TaS Tool
This tool is derived from https://github.com/oberien/refunct-tas.
There is a readme there too.
