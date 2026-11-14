[한국어](install.md)
[English](install_en_US.md)

# Installation
1. Install [Visual Studio Code](https://code.visualstudio.com/Download) and [PlatformIO](https://platformio.org/install/ide?install=vscode).
1. Install [Git](https://git-scm.com/downloads).
1. Connect the ESP32 board and first do `Upload Filesystem Image` in PlatformIO on the left. If you are in `Monitoring` status, please exit.
1. Click the `Upload` button at the bottom to install the firmware.

If the development board uses a USB chipset such as CH340 or CH9102, you must install the driver to connect.

The ESP32-S3 uses USB CDC. While the default Windows driver should work to some extent, there are limitations to normal use. Therefore, it's recommended to install the driver using [Zadig](https://zadig.akeo.ie/). Simply install the USB Serial (CDC) driver on Interface 0.



# Configuration
ToasterFirmware uses FFAT, a lightweight file system, to store data files in Flash.
When doing `Upload Filesystem Image`, all files in the `/data` folder will be uploaded, and the full path name can be up to 31 characters long(up to /123456789012345678901234567890), so please be careful.

Among them, most settings are managed in the `config.yaml` file. Basically, it follows the syntax of yaml, but it does not include all functions, so please format it as best as possible.

If you're using the ESP32-S3, please refer to the contents of the `config_s3.yaml` file. Uploading without changing the file name or contents will result in an error.



# Create a Personality
## Colors
You can easily change colors without modifying the included assets.

```yaml
my_protogen:
  color_eyes: '#00ff00'
  color_nose: color_eyes
  color_mouth: color_eyes
  color_side: '#0000ff'

  # ...
```

You can specify the color for eyes, nose, mouth, and side panel, and it is also possible to apply the same color to other parts as shown in the example.

It uses HTML color codes, which can be checked in Windows Paint or [here](https://www.w3schools.com/colors/colors_picker.asp).


## Default Emotion
Specifies the default emotion to display when turned on.

```yaml
my_protogen:
  # ...
  
  # default emotion
  eyes_default: eyes_normal
  nose_default: nose_default
  mouth_default: mouth_default
  side_default: side_default
```

You can use the file name in the `script` folder for the element to be displayed.



# Create Emotions
Emotions can be created using png images. You can make the eyes/nose/mouth separately, or you can make them as a single picture.

If you create multiple frames in full screen, it may not be displayed due to memory issues.


## Preparing assets
Please prepare a images. Just save in 8-bit RGBA PNG format, up to 64x32 pixels.

You can put the prepared image in the `png` folder.


## Script
Each script is saved in the `script` folder, and each file has the structure below.

```yaml
version: 1
images:
  - /png/full/bsod.png
sequences:
  # draw, IMAGE_NO, COLOR, MODE, OFFSET_X, OFFSET_Y, TIME
  - draw, 0, original, mirror_only_offset, 12, 0, 0
```

- `version`: The script version. It exists for future upgrades. Please set it to 1.
- `images`: List of images to use. After setting it using the file name, it is used as the image number, and the first image is number 0.
- `sequences`: List of images to cycle through.
  - `FUNCTION`: Function.
    - `draw`: Draw without rotating the picture.
    - `draw_90`: Draw the picture by rotating it 90 degrees clockwise.
    - `draw_180`: Draw the picture by rotating it 180 degrees.
    - `draw_270`: Draw the picture by rotating it 270 degrees clockwise.
  - `IMAGE_NO`: The image number to draw. Use the image loaded `images`. The first image is number 0.
  - `COLOR`: Color mode. Applies only when using personal mode.
    - `original`: Displays the image in its original color.
    - `eyes`: Displays the image in eyes color.
    - `nose`: Displays the image in nose color.
    - `mouth`: Displays the image in mouth color.
    - `side`: Displays the image in side panel color.
  - `MODE`: Draw mode. In general, `mirror` and `mirror_only_offset` are often used.
    - `single`: Draw on one side only. To draw in the right panel, just start with an X offset of 64.
    - `copy`: Draw as is on both sides.
    - `mirror`: Draw symmetrically on both sides. (suitable for drawing emotions)
    - `mirror_only_offset`: Draw only the positions on both sides symmetrically. (suitable for displaying images that do not flip like BSOD)
  - `OFFSET_X`: The X offset at which to start displaying the picture. The left is the origin, and it increases as you go to the right.
  - `OFFSET_Y`: The Y offset at which to start displaying the picture. The top is the origin, and it increases as you go to the down.
  - `TIME`: The time to display the image, in seconds. When it times out, it switches to the next line of images. If you are using a single image, just set it to 0.

Since it may be difficult to create the script yourself, I recommend that you copy and use a script with a similar purpose from the examples below.


### Example - Single Image Display
Displays the BSOD.

```yaml
version: 1
images:
  - /png/full/bsod.png
sequences:
  # draw, IMAGE_NO, COLOR, MODE, OFFSET_X, OFFSET_Y, TIME
  - draw, 0, original, mirror_only_offset, 12, 0, 0
```


### Example - 4 Images Round Trip Display
Shows eye blinking. It displays in the following order: 0, 1, 2, 3, 2, 1 and then cycles from the beginning.

```yaml
version: 1
images:
  - /png/eyes/eye_default_0.png
  - /png/eyes/eye_default_1.png
  - /png/eyes/eye_default_2.png
  - /png/eyes/eye_default_3.png
sequences:
  # draw, IMAGE_NO, COLOR, MODE, OFFSET_X, OFFSET_Y, TIME
  - draw, 0, eyes, mirror, 0, 0, 3.0
  - draw, 1, eyes, mirror, 0, 0, 0.05
  - draw, 2, eyes, mirror, 0, 0, 0.05
  - draw, 3, eyes, mirror, 0, 0, 0.05
  - draw, 2, eyes, mirror, 0, 0, 0.05
  - draw, 1, eyes, mirror, 0, 0, 0.05
```


## Registration Emotions
Register emotions in the `config.yaml` file.

```yaml
emotions:
  # ...
  
  - name: my_emotion
    eyes: my_eye
    nose: my_nose
    mouth: my_mouth
    side: my_side
```

You can register all `eyes`, `nose`, `mouth`, and `side`. You can register only some parts, and the omitted parts use the `default` element.



# Upload
Changing any settings does not take effect immediately; you must upload a file to Filesystem for it to take effect.

Connect the ESP32 board and do `Upload Filesystem Image` first in PlatformIO on the left. If you are in `Monitoring` status, please exit.

On the ESP32-S3, you can also edit files directly in [USB storage mode](#usb-storage-mode).



# SD Card
The ESP32-S3 supports using an SD card. This allows you to store and play much larger videos.

If there are duplicate emotions on the FFAT and SD card, the SD card will be used first.

If the SD card isn't recognized, format it with the file system set to `FAT32` and the allocation unit size set to `4096 bytes` and try again.

Cards formatted as `exFAT` or `NTFS` cannot be read. Note that Windows formats cards larger than 32GB as `exFAT` by default.



# USB Storage Mode
> NOTE: This is a beta feature. Keep a backup of important files on your PC.

On the ESP32-S3, the internal flash (FFAT) can be connected to a PC as a USB drive. You can edit `config.yaml` or emotion files directly in your file manager without `Upload Filesystem Image`.

If an [SD card](#sd-card) is inserted, it appears as a separate drive as well.

## Build
USB storage mode requires a separate build environment. Select `env:adafruit_matrixportal_esp32s3_usbm` in the environment selector at the bottom of PlatformIO, then click `Upload`.

This environment uses USB differently (TinyUSB instead of USB-CDC).
- The serial monitor still works, but the COM port number may change after uploading.
- The connection drops every time the firmware restarts, so early boot messages are hard to see.
- The built-in USB JTAG debugging is not available.
- If uploading fails, hold the `BOOT` button and press the `RESET` button to enter download mode, then try again.

## Usage
1. Select `ok` in `Options` > `USB` in the HUD, or enter the serial command `usb`.
1. All LEDs turn off and the device reboots into USB storage mode.
1. Connect it to a PC over USB and a drive appears. Copy or edit files.
1. When you are done, **eject** the drive in your file manager. The device reboots automatically and applies the changes.
   - If the SD card appeared too, ejecting either drive is enough. If the other drive is still being written to, the device reboots after the writes finish, showing `Waiting...` on the HUD in the meantime.

- If you have a HUD, the USB storage mode screen shows the connection status and the amount read/written (total of all drives).
- Unplugging USB without ejecting also powers off or reboots the device within 3 seconds, but data the PC has not finished writing may be lost and files may be corrupted. Always eject first.
- The device also treats the PC going to sleep as a disconnect and reboots.
- If you entered USB storage mode without a USB connection, turn the power off and on to boot normally.
- Writing is slow due to the nature of flash memory. Folders with many small files may take a long time to copy.
- If files are corrupted and cause read errors:
  - Write them again with `Upload Filesystem Image`.
  - Or format the drive with file system `FAT` and allocation unit size `4096 bytes`, then copy the data back.
- SD card:
  - Only a card inserted before entering USB storage mode appears. Do not remove or insert the card while in USB storage mode.
  - A card that cannot be read still appears as a drive, so you can format it on the PC. Be sure to choose `FAT32` as the file system.
  - If the SD card cannot be initialized, the HUD shows `SD card error` and the drive appears empty.
  - Transfers over USB are slow, so for large files it is faster to remove the card and use the PC's card reader.

## Automatic entry on configuration errors
In the USB storage mode build, if the device fails to boot because of an error in `config.yaml` or similar, it enters USB storage mode automatically. Connect it to a PC, fix the files, and eject the drive to boot again.



# Debug Mode
> NOTE: This feature is subject to change at any time!

If connected to the ESP32 board via USB, it can also be operated from a PC. This can be useful when the controller is not ready or for debugging purposes.

Click the `Platformio: Serial Monitor` button at the bottom of the Visual Studio Code screen to enter monitoring mode. You can operate it by pressing the key on this screen.


## Change Emotions
| Key | Function | Note |
| - | - | - |
| 0 | Turn off screen | |
| 1 | Normal (normal) | |
| 2 | Badass (badass) | |
| 3 | Grin (grin) | |
| 4 | Worry (worry) | |
| 5 | Love (love) | |
| 6 | Confusion (confusion) | |
| 7 | Question mark (qmark) | |
| 8 | Cute (unyuu) | |
| 9 | Bean (bean) | |
| P | BSOD (bsod) | |
| W | White screen (white) | When used at maximum brightness, the display may not be displayed properly due to power issues. |
| O | Music visualizer emulation (Party mode) (festive) | |
| ' | Loading (loading) | |


## Glove Controller Emulation
Actions are case-sensitive.
- Lower case: short click
- Upper case: long click

| Key | Function | Note |
| - | - | - |
| F    | Left index finger |  |
| D    | Left middle finger |  |
| S    | Left ring finger |  |
| A    | Left littler finger | Unused, reserved |
| J    | Right index finger |  |
| K    | Right middle finger |  |
| L    | Right ring finger |  |
| ;(:) | Right little finger | Unused, reserved |


## Specials
| Key | Function | Note |
| - | - | - |
| Z | Color mode - Original (original) | |
| X | Color mode - Personal(personal) | |
| C | Color mode - Rainbow(single) (rainbow_single) | |
| V | Color mode - Rainbow (rainbow) | |
| B | Color mode - Gradation (gradation) | |
| E | Toggle Photo mode | Mode to stop screen changes |
| LEFT | Left | Use in menu (back) |
| DOWN | Down | Use in menu |
| UP | Up | Use in menu |
| RIGHT | Right | Use in menu (select) |
| M | Boop sensor debug | Toggle Boop sensor value(mm) through serial |
| \ | Toggle HUD dithering | |
| TAB | Change to next emotion | Quickly change to the next emotion within the registered |


## Commands
Press the `Enter` key or `!` key and enter the command.

| Command | Function | Note |
| - | - | - |
| `help`                 | Display information about builtin commands. | |
| `reset`, `restart` or `reboot` | Soft reset | |
| `b`                    | Set the brightness to that value (0-100). With a light sensor, this pauses auto brightness; `b auto` turns it back on. | ex: `b 100`, `b auto` |
| `ls` or `emotions`     | Show list of loaded emotions | |
| `set`                  | Change to the entered emotion. | ex: `set normal` |
| `mac`                  | Display mac address. | |
| `boop`                 | Enable boop sensor. | |
| `noboop`               | Disable boop sensor. | |
| `dithering`            | Enable HUD dithering. | |
| `nodithering`          | Disable HUD dithering. | |
| `datetime`             | Displays the date and time of the built-in clock. | |
| `settime`              | Sets the time of the built-in clock. | ex: `settime 12:34` or `settime 12:34:56` |
| `setdatetime`          | Sets the date and time of the built-in clock. | ex: `setdatetime 2023-12-08 12:34:56` |
| `color`                | Change the color preset. | ex: `color personal2` |
| `usb`                  | Reboot into USB storage mode. | Only in the [USB storage mode](#usb-storage-mode) build |
| `loglevel`             | Show or change the log level. | ex: `loglevel`, `loglevel info`, `loglevel 3`. See [Log Level](#log-level) |


## Log Level
You can choose how much is logged to the serial monitor. Only logs at the selected level or more important are printed.

| Level | Number | Logs printed |
| - | - | - |
| `none`    | 0 | Nothing |
| `error`   | 1 | Errors |
| `warn`    | 2 | Errors, warnings |
| `info`    | 3 | Errors, warnings, info |
| `debug`   | 4 | Errors, warnings, info, debug (default) |
| `verbose` | 5 | All logs |

The level at boot is set with `log_level` in `config.yaml`. Boot logs printed before the settings are read use the default (`debug`).
```yaml
log_level: info
```

You can change it at runtime with the `loglevel` command. It goes back to the value in `config.yaml` after a reboot.

Logs above the build flag `TF_LOG_MAX_LEVEL` are not included in the firmware, so raising the level will not print them. The default is `4` (`debug`). To see `verbose` logs, add the following to `build_flags` in `platformio.ini` and upload again.
```ini
  -DTF_LOG_MAX_LEVEL=5
```
