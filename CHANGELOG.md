# Changelog


## Unreleased
### 중요
- (ESP32-S3) 이전 버전에서 펌웨어를 업로드한 적이 있다면, 이번 버전을 업로드한 뒤 `Upload Filesystem Image`로 파일을 다시 올리는 것을 권장합니다. 업로드 과정에서 FFAT의 일부 파일이 손상되었을 수 있습니다.
- YAML 들여쓰기 검사가 엄격해져, 이전에는 읽히던 파일이 이제 `indent error`로 처리될 수 있습니다. 목록 항목은 키보다 한 단계 들여써야 합니다.

### 기능
- (Beta) USB 저장소 모드 추가 (ESP32-S3 전용, `adafruit_matrixportal_esp32s3_usbm` 빌드)
  - PC에 USB로 연결하면 내장 플래시(FFAT)가 USB 드라이브로 나타나, `Upload Filesystem Image` 없이 탐색기에서 파일을 바로 고칠 수 있습니다.
  - HUD의 `Options` > `USB` 또는 시리얼 명령 `usb`로 진입합니다. PC에서 꺼내기를 하거나 USB 연결을 해제하면 재부팅합니다.
  - 설정 오류로 부팅하지 못하면 USB 저장소 모드로 자동 진입합니다.
  - SD 카드가 있으면 SD 카드도 별도의 드라이브로 함께 나타납니다. 둘 중 하나만 꺼내도 재부팅하며, 다른 드라이브에 쓰는 중이면 쓰기가 끝날 때까지 기다립니다.
- 시리얼 로그 레벨 설정 추가
  - `config.yaml`의 `log_level`(`none`, `error`, `warn`, `info`, `debug`, `verbose`, 기본값: `debug`)로 설정하고, 실행 중에는 시리얼 명령 `loglevel`로 확인하거나 바꿀 수 있습니다.
  - 빌드 플래그 `-DTF_LOG_MAX_LEVEL=<0-5>`(기본값: `4`, debug)보다 높은 레벨의 로그는 펌웨어에서 제외됩니다.

### 버그 수정
- (ESP32-S3) 펌웨어를 업로드하면 FFAT의 일부 파일이 손상될 수 있던 문제 수정
  - 보드 정의가 업로드 시 TinyUF2 이미지를 함께 기록했는데, 그 위치가 FFAT 파티션 안이라 약 2MB 지점 이후 172KB의 데이터를 덮어썼습니다.
- 연속된 이미지 영상이 재생되지 않던 문제 수정 (`offset: auto`, 시작 프레임, `fps`, 마지막 프레임)
- 오른쪽 개인 색상(`color_right_*`)과 오른쪽 그라데이션(`gradation_right_points`)이 적용되지 않던 문제 수정
- 색상 모드를 적용한 영상(`video: color`)의 오른쪽이 어둡게 표시되던 문제 수정
- 다른 그룹이나 SD 카드에 이름이 같은 스크립트가 있으면 이전 스크립트가 그대로 표시되던 문제 수정
- 스크립트 오류가 한 번 나면 그 부위(눈, 코, 입 등)가 재부팅 전까지 표시되지 않던 문제 수정
- 조도센서 사용 중 직접 조절한 밝기가 곧바로 자동 밝기로 덮어써지던 문제 수정
  - 이제 밝기를 직접 조절하면 자동 밝기가 꺼집니다. HUD의 `Options` > `Auto B.` 또는 시리얼 명령 `b auto`로 다시 켤 수 있으며, 재부팅하면 자동 밝기로 시작합니다.
- 구버전 설정 키 `hardware:i2c_esp_now`로는 ESP-NOW 수신기가 동작하지 않던 문제 수정
- RTC 없이 `clock` 표정을 고르면 시계가 비정상적으로 빨리 흐르던 문제 수정
- Boop 센서 응답이 끊겨도 감지하지 못하던 문제 수정. 측정이 0.5초 이상 끊기면 센서를 다시 초기화합니다.
- 셔플 후 HUD에서 이전 표정이 강조 표시되던 문제 수정
- SD 카드의 표정이 기본 표정을 대체하면 HUD 메뉴에 중복으로 표시되던 문제 수정
- 설정 값이 0이거나 단위가 빠졌을 때 재부팅되거나 멈추던 문제 수정
  - 대상: `rainbow_speed`, `rainbow_single_speed`(예: `10` 대신 `10ms`), `hardware:hub75:fps`, `hardware:hud:fps`, `hardware:thermometer:period`, 스크립트의 `video:fps`
  - 잘못된 값은 경고 로그를 남기고 기본값을 사용합니다.
- 프레임 지연이 0인 GIF를 재생할 때 HUD가 멈출 수 있던 문제 수정
- YAML 파일의 들여쓰기에 따라 항목이 엉뚱한 위치에 들어가거나 재부팅되던 문제 수정
  - 한 줄짜리 목록 항목(`- name: a`)이 연속될 때, 하위 항목을 모두 주석 처리한 키 다음에 다른 키가 올 때 등
  - 값이 있는 키(`key: value`) 아래에 하위 항목을 쓰는 등 들여쓰기가 맞지 않으면 이제 `indent error`로 처리합니다.
  - 목록 항목은 키보다 한 단계 들여써야 합니다. 같은 깊이에 쓰면 오류로 처리합니다.
- MJPEG 영상 관련 문제 수정
  - (PSRAM 있는 보드) 재생을 시작하면 첫 프레임이 약 1초 동안 멈춰 있고 이후 재생이 늦게 따라가던 문제
  - (PSRAM 있는 보드) 디코드가 실패하거나 영상을 바꿀 때 멈출 수 있던 문제
  - (PSRAM 없는 보드) 반복 재생하는 영상이 끝에 도달하면 재부팅될 수 있던 문제
  - 마지막 프레임의 끝 부분이 잘리던 문제
- 시퀀스에서 재생하는 영상 관련 문제 수정
  - 시퀀스의 `video`/`video_loop`로 재생한 MJPEG가 멈추거나 다음 단계로 넘어가지 않던 문제. 시퀀스에는 fps를 지정할 수 없으므로 15fps로 재생합니다.
  - 효과가 여러 개인 표정에서 시퀀스의 `video`(한 번 재생)가 영상 속도를 무시하고 화면 갱신마다 다음 프레임으로 넘어가던 문제
  - 시퀀스의 `video`로 재생한 GIF가 한 번 재생된 뒤 다시 재생되지 않던 문제
  - 이제 한 번 재생(`video`)은 끝나면 다음 단계로 넘어가고, 다시 돌아오면 처음부터 재생합니다.
- 잘못된 설정·스크립트·명령이나 손상된 파일 때문에 재부팅되거나 오동작하던 문제 수정
  - 사이드 패널의 `num_leds`를 생략하거나 1 이하로 쓰고 `festive`, `dino` 표정을 고른 경우
  - `emotion.yaml`의 `default` 표정이 없거나 키가 빠진 경우. 이제 빠진 키는 비워 두고 경고 로그를 남기며, 나머지 표정은 정상적으로 불러옵니다.
  - 스크립트의 이미지 번호가 0-255를 벗어나거나 `images`, `sequences`, `boop_sequences` 항목을 하위 항목으로 쓴 경우, 손상된 GIF를 재생한 경우. 이제 스크립트 오류로 처리합니다.
  - 존재하지 않거나 불러오지 못한 빠른 표정 변경 프리셋을 고른 경우
  - `setdatetime`에 형식이 틀린 값을 넣은 경우. `settime`, `setdatetime`은 범위를 벗어난 값도 거부합니다.
- 메모리 손상, 재부팅, 입력 오동작 등 안정성 문제 수정
  - ESP-NOW 수신, 이미지 그리기, 이미지·영상 로드 실패 시 메모리 손상
  - 리모컨(ESP-NOW, IR)과 시리얼에서 동시에 키를 입력하거나 HUD가 화면을 바꾸는 중에 키를 입력할 때 HUD 메뉴 오동작
  - ESP-NOW 리모컨으로 셔플할 때 표정을 불러오는 동안 리모컨 수신이 멈추던 문제
  - 공룡 게임의 메모리 누수, 사이드 패널 LED가 37개를 넘을 때 원형 무지개 효과의 메모리 범위 밖 읽기

### 기타
- RTC(DS3231) 사용 시 10분마다 시간을 다시 맞추도록 변경
- 기본 `eyelash` 표정의 중복된 `mouth` 키 정리
- 여러 작업에서 동시에 출력한 로그가 한 줄 안에서 섞이지 않도록 개선
- PC에서 실행하는 단위 테스트 추가 (`test/host`, YAML 파서와 문자열 함수)
- 외부 라이브러리 버전 고정 (`platformio.ini`의 `lib_deps`)
- (ESP32-S3) 불필요한 빌드 플래그 `-mfix-esp32-psram-cache-issue` 제거
- HUD 화면 미러링, GIF 디코딩, 설정 파일 읽기의 메모리 할당을 줄이고, 영상·이미지·설정 파일을 불러올 때 메모리가 부족하면 재부팅하지 않고 오류로 처리하도록 개선
- (PSRAM 있는 보드) MJPEG 영상을 64KB 단위로 미리 읽어 두고 프레임을 찾도록 바꿔 파일 읽기와 메모리 할당을 줄임. 64KB를 넘는 프레임은 경고 로그를 남기고 건너뜁니다.

### Important
- (ESP32-S3) If you uploaded the firmware with an earlier version, it is recommended to upload this version and then re-upload your files with `Upload Filesystem Image`. Some files in FFAT may have been corrupted during the upload.
- YAML indentation is now checked more strictly, so files that loaded before may now be reported as `indent error`. List items must be indented one level deeper than their key.

### Feature
- (Beta) Added USB storage mode (ESP32-S3 only, `adafruit_matrixportal_esp32s3_usbm` build).
  - When connected to a PC over USB, the internal flash (FFAT) appears as a USB drive, so you can edit files directly in your file manager without `Upload Filesystem Image`.
  - Enter it with `Options` > `USB` in the HUD or the serial command `usb`. Ejecting it on the PC or disconnecting USB reboots the device.
  - If the device fails to boot because of a configuration error, it enters USB storage mode automatically.
  - If an SD card is inserted, it appears as a separate drive as well. Ejecting either drive reboots the device, after waiting for any writes to the other drive to finish.
- Added serial log level setting.
  - Set it with `log_level` in `config.yaml` (`none`, `error`, `warn`, `info`, `debug`, `verbose`, default: `debug`), and check or change it at runtime with the serial command `loglevel`.
  - Logs above the build flag `-DTF_LOG_MAX_LEVEL=<0-5>` (default: `4`, debug) are left out of the firmware.

### Bugfix
- (ESP32-S3) Fixed firmware uploads possibly corrupting some files in FFAT.
  - The board definition also flashed the TinyUF2 image on upload, at an address inside the FFAT partition, overwriting about 172KB of data from around the 2MB point.
- Fixed image sequence videos not playing (`offset: auto`, start frame, `fps`, last frame).
- Fixed the right-side personal colors (`color_right_*`) and right-side gradation (`gradation_right_points`) not being applied.
- Fixed the right side of videos with a color mode (`video: color`) appearing darker.
- Fixed the previous script staying on screen when a script with the same name exists in another group or on the SD card.
- Fixed a part (eyes, nose, mouth, etc.) staying blank until reboot after a single script error.
- Fixed manually set brightness being overwritten by auto brightness right away when using a light sensor.
  - Changing the brightness manually now turns auto brightness off. Turn it back on with `Options` > `Auto B.` in the HUD or the serial command `b auto`. Auto brightness is always on after a reboot.
- Fixed the ESP-NOW receiver not working with the legacy config key `hardware:i2c_esp_now`.
- Fixed the clock running abnormally fast when the `clock` emotion is selected without an RTC.
- Fixed boop sensor dropouts going undetected. The sensor is now reset when no measurement arrives for 0.5 seconds.
- Fixed the HUD highlighting the previous emotion after shuffling.
- Fixed duplicate entries in the HUD menu when an SD card emotion replaces a built-in one.
- Fixed reboots or freezes when a setting is 0 or is missing its unit.
  - Affected settings: `rainbow_speed`, `rainbow_single_speed` (e.g. `10` instead of `10ms`), `hardware:hub75:fps`, `hardware:hud:fps`, `hardware:thermometer:period`, and `video:fps` in scripts.
  - Invalid values are logged as a warning and replaced with the default value.
- Fixed the HUD possibly freezing while playing a GIF with a frame delay of 0.
- Fixed YAML items ending up in the wrong place, or a reboot, depending on indentation.
  - For example, consecutive single-line list items (`- name: a`), or a key that follows another key whose children are all commented out.
  - Mismatched indentation, such as children under a key that has a value (`key: value`), is now reported as `indent error`.
  - List items must be indented one level deeper than their key. Items at the same level are reported as an error.
- Fixed MJPEG video issues.
  - (Boards with PSRAM) The first frame stayed on screen for about a second when playback started, and playback lagged behind after that.
  - (Boards with PSRAM) Playback could freeze when decoding failed or when switching videos.
  - (Boards without PSRAM) A looping video could cause a reboot when it reached the end.
  - The end of the last frame was cut off.
- Fixed issues with videos played in a sequence.
  - MJPEG videos played with `video`/`video_loop` in a sequence froze or did not move on to the next step. Since a sequence cannot specify fps, they now play at 15fps.
  - In emotions with more than one effect, a `video` step in a sequence ignored the video's speed and advanced a frame on every screen refresh.
  - GIFs played with `video` in a sequence did not play again after the first time.
  - A `video` step now moves on when the video ends, and plays from the beginning the next time it comes around.
- Fixed reboots or misbehavior caused by invalid settings, scripts, or commands, or by corrupted files.
  - Choosing the `festive` or `dino` emotion with the side panel's `num_leds` omitted or set to 1 or less.
  - A missing or incomplete `default` emotion in `emotion.yaml`. Missing keys are now left empty with a warning, and the other emotions load normally.
  - Image numbers outside 0-255 in a script, items in `images`, `sequences`, or `boop_sequences` written as nested items, or a corrupted GIF. These are now reported as a script error.
  - Choosing an emotion shortcut preset that does not exist or fails to load.
  - A malformed value given to `setdatetime`. `settime` and `setdatetime` also reject out-of-range values.
- Fixed stability issues such as memory corruption, reboots, and input glitches.
  - Memory corruption in ESP-NOW receiving, image drawing, and failed image/video loading.
  - The HUD menu misbehaving when keys arrived from the remotes (ESP-NOW, IR) and serial at the same time, or while the HUD was changing screens.
  - ESP-NOW remote input stalling while an emotion loaded after a shuffle from the remote.
  - A memory leak in the dino game, and an out-of-bounds read in the circular rainbow effect when the side panel has more than 37 LEDs.

### Misc.
- With an RTC (DS3231), the time is re-synchronized every 10 minutes.
- Removed duplicate `mouth` keys in the built-in `eyelash` emotions.
- Logs printed at the same time from multiple tasks no longer get mixed within a line.
- Added unit tests that run on a PC (`test/host`, YAML parser and string utilities).
- Pinned the versions of external libraries (`lib_deps` in `platformio.ini`).
- (ESP32-S3) Removed the unnecessary build flag `-mfix-esp32-psram-cache-issue`.
- Reduced memory allocations in HUD mirroring, GIF decoding, and config file loading, and running out of memory while loading videos, images, or config files is now handled as an error instead of rebooting.
- (Boards with PSRAM) MJPEG videos are now read ahead in 64KB chunks to find frames, reducing file reads and memory allocations. Frames larger than 64KB are skipped with a warning.


## 2026.1.1
### 기능
- ESP32-S3 지원 추가. `Adafruit MatrixPortal S3` 보드를 정식 지원합니다.
- PSRAM 지원 추가
  - 외부의 보드 없이 ESP-NOW 지원 추가
  - SD 카드 리더 지원 추가. 용량 부족으로 재생하지 못하던 수 많은 영상들을 마음껏 뽐내주세요!
  - (Beta) 영상 처리 로직 개선 (42x32 영상 기준 GIF 최대 50fps, MJPEG 최대 60fps 확인)
- HUB75 패널 `min_refresh_rate` 상향. 이제 사진 촬영 시 플리커링을 더 줄일 수 있으며 최대 1220Hz까지 지원됩니다.
- 개인 색상 설정 3개로 개선 (`personal2`, `personal3`)
- 시간 설정 메뉴 추가
- 기본 표정 3종이 추가되었습니다. (default: `ded`, `angy`, `bean2`)

### 버그 수정
- BH1750 설정 안 되는 버그 수정
- MJPEG 로딩 실패 시 메모리 누수 수정

### 기타
- 비활성화 Timeout 게이지 모든 메뉴에 추가
- 비활성화 Timeout 조정
- 빠른 표정 변경 기능에서 표정 이름 미리보기 추가
- IRremoteESP8266 컴파일 속도 개선 (NEC만 남기고 제거된 라이브러리로 교체)

### Feature
- Added ESP32-S3 support. Officially supports the `Adafruit MatrixPortal S3` board.
- Added PSRAM support.
  - Added ESP-NOW support without an external board.
  - Added SD card reader support. Show off all the videos you couldn't play due to insufficient storage!
  - (Beta) Improved video processing logic (up to 50fps for GIFs, up to 60fps for MJPEGs at 42x32)
- Increased `min_refresh_rate` for the HUB75 panel. This further reduces flickering when taking photos and supports up to 1220Hz.
- Improved personal color settings to 3 (`personal2`, `personal3`)
- Added a time setting menu.
- 3 emotions added. (default: `ded`, `angy`, `bean2`)

### Bugfix
- Fixed a bug that prevented BH1750 from being set up.
- Fixed a memory leak when MJPEG loading failed.

### Misc.
- Added a Inactive Timeout Gauge to all menus
- Adjusted Inactive Timeout
- Added a preview of the emotion name in the Emotion Shortcut
- Improved compilation speed for IRremoteESP8266 (replaced with the removed library, leaving only NEC)


## 2025.3.1
### 주의 사항
- 빠른 표정 변경 설정을 저장하는 폴더 구조가 변경되었습니다. 여러 프리셋을 이용하여 표정을 더 편하게 바꾸세요! (`/shortcut`)

### 기능
- 그 공룡 게임을 할 수 있습니다. Boop 센서를 활용해 점프하세요! (유진님 아이디어 제공 고맙습니다!!!)
- 빛 퍼짐이 개선된 사이드 패널 V2를 지원합니다.
- 기본 표정 5종이 추가되었습니다. (default: `closed`, `smirk`, eyelash: `normal_blushing_e`, `closed_e`, `smirk_e`)
- 이제 얼굴색을 좌/우 다르게 설정 할 수 있습니다. (`color_right_eyes`, `color_right_nose`, `color_right_mouth`, `color_right_side`)
- 그라데이션도 좌/우 다르게 설정 할 수 있도록 변경 (`my_protogen:gradation_right_points`)
- 사이드 패널 LED 순서를 반대로 사용 할 수 있는 설정 추가 (`hardware:side_panel:reversed`)
- 표정 셔플 기능이 추가되었습니다. 어떤 액팅을 할 지 고민될 때 사용해보세요!
- 빠른 표정 변경 기능에 프리셋이 추가되었습니다. 이제 여러 프리셋을 이용하여 더욱 편하게 사용할 수 있습니다.
- Uptime 표시가 추가되어 퍼슈팅을 한 시간을 편하게 확인 할 수 있습니다. 또한 DS3231 RTC를 연결하여 시간을 확인 할 수 있습니다.
- 투명도가 없는 흑백 동영상도 무지개빛으로 재생 할 수 있는 기능이 추가되었습니다.

### 버그 수정
- IR 리모컨으로 사진 모드 동작하지 않는 문제 수정 (Fallen_Servoh 님 고맙습니다!)
- 동시 동작하는 스크립트간 시간 편차가 생기는 문제 수정
- 밝기 조절 메뉴에서 장갑형 컨트롤러로 빠져나갈 수 없는 문제 수정
- 밝기 조절 메뉴에서 시간 초과시 빠져나가지 않는 문제 수정
- 밝기 조절 메뉴에서 밝기 조절시 사이드 패널이 올바르지 않은 밝기로 조절되는 문제 수정
- 특정 조건의 gif 파일이 정상적으로 표시되지 않는 문제 수정
- 간헐적으로 프로그램이 멈추는 문제 해결

### 기타
- HUD 폰트 개선
- `Hub75Display::draw_image_newcolor` 코드 간결하게 수정
- 밝기 조절 메뉴 위치 이동
- ESP-Now 장갑 컨트롤러 프로토콜 수정

### CAUTION
- The folder structure for saving Emotion Shortcut settings has been changed. Change your emotion more easily using multiple presets! (`/shortcut`)

### Feature
- You can play that dinosaur game. Jump using the Boop sensor! (Thank you for the idea, Yujin!!!)
- Supports Side Panel V2 with improved light diffusion.
- 5 emotions added. (default: `closed`, `smirk`, eyelash: `normal_blushing_e`, `closed_e`, `smirk_e`, )
- You can set the left/right face color differently. (`color_right_eyes`, `color_right_nose`, `color_right_mouth`, `color_right_side`)
- You can also set the the gradient to be set differently for left/right (`my_protogen:gradation_right_points`)
- Added a setting to reverse the order of the side panel LEDs (`hardware:side_panel:reversed`)
- Emotion shuffle function added. Use it when you are wondering what kind of acting to do!
- Presets added to the quick emotion change function. Now you can use it more conveniently with multiple presets.
- Uptime display added so you can easily check the time of the fursuit. You can also check the time by connecting the DS3231 RTC.
- Added a function to play black and white video without transparency in rainbow colors.

### Bugfix
- Fixed an issue where photo mode did not work with IR remote control (Thank you, Fallen_Servoh!)
- Fixed an issue where there was a time difference between concurrently running scripts
- Fixed an issue where the brightness control menu could not be exited with a glove controller
- Fixed an issue where the brightness control menu could not be exited when timed out
- Fixed an issue where the side panel was adjusted to an incorrect brightness when adjusting brightness in the brightness control menu
- Fixed an issue where gif files under certain conditions were not displayed properly
- Fixed an issue where the program would intermittently freeze

### Misc.
- Improved HUD font
- Simplified `Hub75Display::draw_image_newcolor` code
- Moved brightness control menu location
- Modified ESP-Now glove controller protocol


## 2024.5.2
### 주의 사항
- 표정을 저장하는 폴더 구조가 변경되었습니다. 이제 다른 사람에게 공유받은 표정을 더 편리하게 추가하세요! (`/emotions`)

### 기능
- GIF, MJPEG 지원
- 표정에 동영상을 추가 할 수 있도록 명령어 추가 (`video`, `video_loop`)
- HUD 대시보드 디자인 변경 (영문자 대신 직관적인 아이콘으로 변경)
- HUD 디더링 지원 추가 (`hardware:hud:dithering`)
- 기본 표정의 눈 크기 조정, 프레임 향상
- 속눈썹이 존재하는 표정 그룹 추가 (`eyelash`)
- 표정에 특수 효과 지원을 위한 레이어 2개 추가 (`special`, `special2`)
- ESP-Now 장갑 컨트롤러 지원 (베타)
- 빠른 표정 변경 모드 추가 (`/emotions/shortcut.yaml`)

### 버그 수정
- 동적 이미지 로딩 기능이 제대로 동작하지 않는 문제 수정
- 표정 이름이 긴 경우 메뉴에 제대로 표시되지 않는 문제 수정
- 특정 조건에서 Core 1 Task 시작에 실패하는 문제 수정

### 기타
- Splash 화면 skip 가능하도록 변경
- 시리얼 모니터에 명령어 입력시 피드백 추가
- 내부 이미지 버퍼 RGB888 대신 RGB565로 사용하도록 변경 (`hardware:hub75:use_rgb565`)
- 화면 주사율 변경시 HUD도 변경되도록 수정

### CAUTION
- The folder structure for storing emotion has changed. Now you can more conveniently add emotions shared by others! (`/emotions`)

### Feature
- Add support for GIF, MJPEG
- Add support for command in emotion (`video`, `video_loop`)
- HUD dashboard design change (change to icons instead of character)
- Add support dithering for HUD (`hardware:hud:dithering`)
- Adjusted eye size and improved framing for basic emotions
- Add emotions group with eyelashes (`eyelash`)
- Add 2 layers for special effect (`special`, `special2`)
- Add support for ESP-Now glove controller (Beta)
- Add Emotion Shortcut

### Bugfix
- Fix dynamic image loading do not work as intended
- Fix long emotion names were not displayed properly in the menu
- Fix Core 1 Task fails to start under certain conditions

### Misc.
- Changed Splash screen to be skippable
- Added feedback when entering commands in the serial monitor
- Changed internal image buffer to use RGB565 instead of RGB888. (`hardware:hub75:use_rgb565`)
- HUD refresh rate also changes when the screen refresh rate changes.


## 2024.5.1
### 기능
- 사진 촬영시 플리커링 감소를 위해, HUB75 패널 min_refresh_rate 변경 (60 -> 240)

### Feature
- To reduce flickering when taking photos, HUB75 panel min_refresh_rate changed (60 to 240)


## 2024.4.1
### 주의 사항
SPIFFS에서 FFAT으로 저장소 형식이 변경되었습니다. 프로그램 뿐만 아니라 파일 시스템도 다시 업로드해야 정상 사용 가능합니다.

### 기능
- Boop 센서 VL53L1X 지원 추가
- 조도센서 BH1750 지원 추가
- 온습도센서 SHT31 지원 추가
- 메인 패널과 사이드 패널 밝기 별도 조절 가능하도록 개선
- 동적 이미지 로딩 기능 추가
  - 이제 이미지가 많아서 표시 할 수 없던 표정도 표시 할 수 있습니다!
  - 대신 60fps 기준 약간의 프레임 드랍이 발생 할 수 있습니다.
- 비디오 모드 추가
  - 일반 스크립트보다 간략화된 새로운 스크립트를 지원합니다.
  - 재생시 화면 주사율이 변경합니다.
  - 30fps 초과 재생시 프레임 드랍이 발생 할 수 있습니다.
  - 모니터에 `!set video`를 입력해서 확인해보세요! (양말님 고마워요!)
- 음악 시각화 에뮬레이션 모드 (파티 모드) 에서 얼굴 표시 여부 설정 추가 (`my_protogen:festive_face`)
- 표정 변화에 걸리는 시간 대폭 개선

### 버그 수정
- 조도센서 사용시 특정 조건에서 최소/최대 밝기로 조절되지 않는 문제 수정
- 최초 전원 인가시 HUD에 깨진 이미지가 표시되는 문제 수정
- 오차 누적으로 미세하게 빨라지는 문제 수정
- 표정 변경 최적화가 동작하지 않는 문제 수정

### 기타
- config.yaml 버전 변경 (1 -> 2)
  - 버전 1도 아직 사용 가능
  - 조도센서 설정 `photoresistor` -> `lightsensor`로 변경
  - 기본 표정 각 부위별(`eyes_default`, `nose_default`, `mouth_default`, `side_default`) -> 전체 표정(`default_emotion`)으로 변경
- 조도센서 처리 코어 0으로 이동
- 로그 표시 개선 (시간, 색상 추가)
- 시리얼을 통한 Soft reset 추가 (`!reset`)
- 시리얼을 통한 표정 변경 명령어 추가
  - `!set [표정]` (예제: `!set video`)
- 파일 시스템 변경 (`SPIFFS` -> `FFAT`)
- 파티션 테이블 변경 (`default.csv` -> `noota_3g.csv`)
  - OTA를 위한 용량 제거 (1.25MB -> 0MB)
  - 애플리케이션 용량 감소 (1.25MB -> 1MB)
  - 저장소 용량 증가 (1.375MB -> 2.875MB)
- HUD 미러링 성능 개선

### CAUTION
Storage format changed from SPIFFS to FFAT. You will need to re-upload not only the program but also the file system.

### Feature
- Add support for VL53L1X, Boop sensor
- Add support for BH1750 light sensor
- Add support for SHT31, temperature/humidity sensor
- Improved the brightness of the main panels and side panels to be adjusted differently
- Dynamic image loading
  - Now you can display emotions that could not be displayed due to too many images!
  - Instead, slight frame drops may occur at 60fps.
- Add video mode
  - Supports new simplified scripts.
  - The screen refresh rate changes during playback.
  - Frame drops exceeding 30fps may occur.
  - Enter `!set video` into monitor and check out! (Thank you, SheepPony!)
- Add face visibility in Music visualizer emulation (`my_protogen:festive_face`)
- Improved emotion change performance

### Bugfix
- Fix the brightness was not adjusted to the minimum/maximum some conditions when using the light sensor
- Fix a broken image was displayed on the HUD when first powering on
- Fix slight speedup due to error accumulation
- Fix an issue where emotion change optimization did not work

### Misc.
- Increase config.yaml version (1 to 2)
  - Version 1 is still compatible
  - `photoresistor` changed to `lightsensor`.
  - Default emotion changed from each(`eyes_default`, `nose_default`, `mouth_default`, `side_default`) to all(`default_emotion`).
- Light sensor processing moved to core 0
- Improved log display (add time and color)
- Add Soft reset via Serial (`!reset`)
- Add set emotion via Serial
  - `!set [emotion]` (ex: `!set video`)
- Change filesystem (`SPIFFS` -> `FFAT`)
- Change flash partition table (`default.csv` to `noota_3g.csv`)
  - Remove space for OTA (1.25MB to 0MB)
  - Less space for Application (1.25MB to 1MB)
  - More space for Storage (1.375MB to 2.875MB)
- Improve performance when HUD mirroring


## 2024.1.1
- 문서화 된 최초 빌드
- First documented build
