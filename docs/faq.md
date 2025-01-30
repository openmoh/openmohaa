# Frequently Asked Questions

## 1. My OpenMoHAA client has crashed, leaving the desktop brightened up. How can I fix this without restarting the computer?

### 1.1 Lock and Unlock Your Screen (Windows+L)
Simply pressing **Win+L** to lock your screen and then logging back in can sometimes reset the brightness to its previous state.

### 1.2. Command Line Fix (Windows)
If you have an **NVIDIA** or **AMD** GPU, you can try these commands:

- **For NVIDIA GPUs:**
  Open **Command Prompt (cmd.exe)** or **PowerShell** and run:
  
  ```powershell
  nvidia-settings -a "DPY-0/RedBrightness=0.0" -a "DPY-0/GreenBrightness=0.0" -a "DPY-0/BlueBrightness=0.0"
  ```
  
  (If you have multiple displays, `DPY-0` may need to be replaced with the correct display identifier.)

- **For AMD GPUs:**
  Open the **AMD Radeon Software** and manually adjust brightness or SDR settings to reset them.

### 1.3. Use a Third-Party Tool to Reset Gamma & Brightness
Programs like **Gamma Panel** or **f.lux** can help restore modified brightness and gamma settings quickly.

### 1.4. Restart the Graphics Driver (Windows)
Press **Win + Ctrl + Shift + B** to restart the graphics driver without rebooting your PC. Your screen will briefly go black, then reset.

### 1.5. Reset Gamma via Xrandr (Linux, Xorg Users)
If you’re using Linux with Xorg, try this command to reset gamma settings:

```bash
xrandr --output eDP-1 --gamma 1:1:1
```

(Replace `eDP-1` with the correct display name, which you can find by running `xrandr` in the terminal.)

### 1.6. Changing Display Mode in Windows (Alternative)
If none of the above solutions work, you can manually reset brightness by changing the display mode:
1. Right-click on your desktop and select **Display settings**.
2. Scroll down to **Display resolution** and change it to a different setting.
3. Revert it back to your original resolution.

This forces Windows to refresh display settings, which may restore brightness.
