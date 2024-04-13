[한국어](install.md)
[English](install_en_US.md)

# Installation
1. Install [Visual Studio Code](https://code.visualstudio.com/Download) and [PlatformIO](https://platformio.org/install/ide?install=vscode).
1. Install [Git](https://git-scm.com/downloads).
1. Connect the ESP32 board and first do `Upload Filesystem Image` in PlatformIO on the left. If you are in `Monitoring` status, please exit.
1. Click the `Upload` button at the bottom to install the firmware.

If the development board uses a USB chipset such as CH340 or CH9102, you must install the driver to connect.



# Configuration
ToasterFirmware uses FFAT, a lightweight file system, to store data files in Flash.
When doing `Upload Filesystem Image`, all files in the `/data` folder will be uploaded, and the full path name can be up to 31 characters long(up to /123456789012345678901234567890), so please be careful.

Among them, most settings are managed in the `config.yaml` file. Basically, it follows the syntax of yaml, but it does not include all functions, so please format it as best as possible.



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
| ; | Music visualizer emulation (Party mode) (festive) | |
| ' | Loading (loading) | |


## Specials
| Key | Function | Note |
| - | - | - |
| Z | Color mode - Original (original) | |
| X | Color mode - Personal(personal) | |
| C | Color mode - Rainbow(single) (rainbow_single) | |
| V | Color mode - Rainbow (rainbow) | |
| B | Color mode - Gradation (gradation) | |
| A | Toggle Photo mode | Mode to stop screen changes |
| H | Left | Use in menu (back) |
| J | Down | Use in menu |
| K | Up | Use in menu |
| L | Right | Use in menu (select) |
| M | Boop sensor debug | Toggle Boop sensor value(mm) through serial |


## Commands
| Command | Function | Note |
| - | - | - |
| `!reset` | Soft Reset | |
| `!b [number 0 ~ 100]` | Set the brightness to that value. | ex: `!b 100` |
| `!set [emotion]` | Change to the entered emotion. | ex: `!set normal` |
| `!noboop` | Disable Boop sensor. | |
| `!boop` | Enable Boop sensor. | |
