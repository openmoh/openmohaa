# Frequently Asked Questions

<details>
<summary>The game won't start or crashes immediately. What should I check?</summary>

## Try the following steps in order:

### 1. Install Microsoft Visual C++ Redistributable (Windows only)
OpenMoHAA requires the Visual C++ Redistributable to run. Download and install it from [here](https://aka.ms/vs/17/release/vc_redist.x64.exe). If you already installed it, try reinstalling it.

### 2. Verify that the game files are in the right place
Make sure you extracted the OpenMoHAA files directly into your MOHAA game folder. You should see:
- The `launch_openmohaa_` executables in the same folder as the `main` subfolder
- The `main` subfolder should contain files like `Pak0.pk3`, `Pak1.pk3`, etc.

If you accidentally extracted into a subfolder (e.g., `MOHAA/openmohaa-0.80.0/`), move the files up one level.

### 3. Check that you have the original game files
OpenMoHAA is not a standalone game - it needs the original MOHAA game files to run. Make sure you have Medal of Honor: Allied Assault installed.

### 4. Check for error messages
Error messages displayed in the terminal are often helpful for diagnosing issues. You can also check the log file located in your user data folder (`%APPDATA%\openmohaa\main` on Windows, `~/.openmohaa/main` on Linux).
</details>

---

<details>
<summary>My OpenMoHAA client has crashed, leaving the desktop brightened up. How can I fix this without restarting the computer?</summary>

## Try one of the following options:
  
### 1. Lock and Unlock Your Screen (Windows+L)
Simply pressing **Win+L** to lock your screen and then logging back in can sometimes reset the brightness to its previous state.

### 2. Command Line Fix (Windows)
If you have an **NVIDIA** or **AMD** GPU, you can try these commands:

- **For NVIDIA GPUs:**
  Open **Command Prompt (cmd.exe)** or **PowerShell** and run:
  
  ```powershell
  nvidia-settings -a "DPY-0/RedBrightness=0.0" -a "DPY-0/GreenBrightness=0.0" -a "DPY-0/BlueBrightness=0.0"
  ```
  
  (If you have multiple displays, `DPY-0` may need to be replaced with the correct display identifier.)

- **For AMD GPUs:**
  Open the **AMD Radeon Software** and manually adjust brightness or SDR settings to reset them.

### 3. Use a Third-Party Tool to Reset Gamma & Brightness
Programs like **Gamma Panel** or **f.lux** can help restore modified brightness and gamma settings quickly.

### 4. Restart the Graphics Driver (Windows)
Press **Win + Ctrl + Shift + B** to restart the graphics driver without rebooting your PC. Your screen will briefly go black, then reset.

### 5. Reset Gamma via Xrandr (Linux, Xorg Users)
If you’re using Linux with Xorg, try this command to reset gamma settings:

```bash
xrandr --output eDP-1 --gamma 1:1:1
```

(Replace `eDP-1` with the correct display name, which you can find by running `xrandr` in the terminal.)

### 6. Changing Display Mode in Windows (Alternative)
If none of the above solutions work, you can manually reset brightness by changing the display mode:
1. Right-click on your desktop and select **Display settings**.
2. Scroll down to **Display resolution** and change it to a different setting.
3. Revert it back to your original resolution.

This forces Windows to refresh display settings, which may restore brightness.
</details>

---

<details>
<summary>I am using a custom map/mod, and I experience glitches that do not occur in the original game. What should I do?</summary>

### 1. Check game file precedences:
As OpenMoHAA has MultiUser Support (on Windows, user game data is stored in `%APPDATA%\openmohaa`), custom files in this directory override existing files in the game installation folder.

|Example|
|-|
| A custom `grenzuebergang_KE.pk3` (placed in the user game data folder) contains a `scripts/effects.shader` file and it does not declare `bh_wood_puff_simple`. As a result, bullet impact effect has no texture.
However, if `grenzuebergang_KE.pk3` is placed in the game installation folder (`MOHAA/main` or `/mainta` or `/maintt`, where the base `pak*.pk3` files are located) original files take precedence over custom files as they follow the alphabetical order of file naming.
`Pak1.pk3` contains the original `scripts/effects.shader` where the impact effect is declared; therefore, the player does not experience any issue, as the original file is "loaded" after the custom file. |

### 2. Create a new issue:
If changing the file precedences has no effect on the glitch you discovered, you have probably found a new bug in the OpenMoHAA engine, so please <a href="https://github.com/openmoh/openmohaa/issues/new/choose">create a new issue</a> for the developers.
</details>

---

<details>
<summary>I cannot set my screen resolution in the Options/Video menu. How can I change it?</summary>

### Edit `omconfig.cfg`:

If they do not exist, add the following console variables (cvars) to your `omconfig.cfg`[^1]

```
seta r_mode "-1"
seta r_customwidth "1920"
seta r_customheight "1080"
```

### or (alternatively) run the game with command line parameters:

Launch your OpenMoHAA client with the following:

```
+set r_mode -1 +set r_customwidth 1920 +set r_customheight 1080
```

Change the width and height accordingly.

</details>

---

<details>
<summary>Console does not show up in OpenMoHAA. How can I enable it?</summary>

### 1. Check if console is enabled

In the Options -> Advanced menu, make sure that the checkbox is checked (red "X") at the Console.
Alternatively, check if the value is equal to 1 for the following cvar in `omconfig.cfg`[^1]:
```
seta ui_console "1"
```

### 2. Check Console keys cvar

OpenMoHAA introduces a new cvar that stores the keys to open up console. Edit the following cvar in your `omconfig.cfg`[^1]:

```
seta cl_consoleKeys "~ ` 0x7e 0x60"
```

As you see the default variable above, you can add multiple keys (between the quotation marks, divided by spaces) to display/hide the console. 

> bind ` "toggleconsole" is not to be used anymore.
</details>


---

Footnotes:

[^1]: omconfig.cfg is the OpenMoHAA configuration file that is located in the user game data folder (on Windows,  `%APPDATA%\openmohaa\main` or `mainta` or `maintt` `\configs\omconfig.cfg`)
