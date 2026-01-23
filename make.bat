@echo off
set "ESC="
for /F %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"

echo Preparing build
if not exist "build" md build
if not exist "build\practice-windows" md build\practice-windows
echo Building

cd rtil
cargo build --release --target=i686-pc-windows-msvc || exit /b
cd ..\tool
cargo build --target=i686-pc-windows-msvc || exit /b
cd ..
echo Copying files to build folder
copy rtil\target\i686-pc-windows-msvc\release\rtil.dll build\practice-windows > NUL
if ERRORLEVEL 1 (
    echo %ESC%[31m  Error: Could not copy rtil.dll file ^(probably still injected into Refunct^)%ESC%[0m
)
copy tool\target\i686-pc-windows-msvc\debug\refunct-tas.exe build\practice-windows > NUL
copy tool\debug.bat build\practice-windows > NUL
copy tool\main.re build\practice-windows > NUL
copy tool\prelude.re build\practice-windows > NUL
copy tool\component.re build\practice-windows > NUL
copy tool\keys.re build\practice-windows > NUL
copy tool\newgame.re build\practice-windows > NUL
copy tool\practice.re build\practice-windows > NUL
copy tool\randomizer.re build\practice-windows > NUL
copy tool\teleport.re build\practice-windows > NUL
copy tool\ui.re build\practice-windows > NUL
copy tool\multiplayer.re build\practice-windows > NUL
copy tool\archipelago_ui.re build\practice-windows > NUL
copy tool\archipelago_gameplay.re build\practice-windows > NUL
copy tool\archipelago_log.re build\practice-windows > NUL
copy tool\archipelago_meta.re build\practice-windows > NUL
copy tool\tas.re build\practice-windows > NUL
copy tool\windshieldwipers.re build\practice-windows > NUL
copy tool\settings.re build\practice-windows > NUL
copy tool\misc.re build\practice-windows > NUL
copy tool\timer.re build\practice-windows > NUL
copy tool\movement.re build\practice-windows > NUL
copy tool\minimap.re build\practice-windows > NUL
copy tool\mapeditor.re build\practice-windows > NUL
copy tool\world_options.re build\practice-windows > NUL
copy tool\player.re build\practice-windows > NUL
copy tool\log.re build\practice-windows > NUL

echo Converting LF to CRLF in Rebo files
call :convert main.re > NUL
call :convert prelude.re > NUL
call :convert component.re > NUL
call :convert keys.re > NUL
call :convert newgame.re > NUL
call :convert practice.re > NUL
call :convert randomizer.re > NUL
call :convert teleport.re > NUL
call :convert ui.re > NUL
call :convert multiplayer.re > NUL
call :convert archipelago_ui.re > NUL
call :convert archipelago_log.re > NUL
call :convert archipelago_meta.re > NUL
call :convert archipelago_gameplay.re > NUL
call :convert tas.re > NUL
call :convert windshieldwipers.re > NUL
call :convert settings.re > NUL
call :convert misc.re > NUL
call :convert timer.re > NUL
call :convert movement.re > NUL
call :convert minimap.re > NUL
call :convert mapeditor.re > NUL
call :convert world_options.re > NUL
call :convert player.re > NUL
call :convert log.re > NUL

echo Don't forget to create a zip
exit /b 0

:convert
type "build\practice-windows\%~1" | find /v "" > "build\practice-windows\%~1.crlf"
move "build\practice-windows\%~1.crlf" "build\practice-windows\%~1"
exit /b 0
