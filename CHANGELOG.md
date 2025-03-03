# Changelog


## 2025.3.1
### 주의 사항
- 빠른 표정 변경 설정을 저장하는 폴더 구조가 변경되었습니다. 여러 프리셋을 이용하여 표정을 더 편하게 바꾸세요! (`/shortcut`)

### CAUTION
- The folder structure for saving quick emotion change settings has been changed. Change your emotion more easily using multiple presets! (`/shortcut`)

### 기능
- 그 공룡 게임을 할 수 있습니다. Boop 센서를 활용해 점프하세요! (유진님 아이디어 제공 고맙습니다!!!)
- 빛 퍼짐이 개선된 사이드 패널 V2를 지원합니다.
- 기본 표정 5종이 추가되었습니다. (default: `closed`, `smirk`, eyelash: `normal_blushing_e`, `closed_e`, `smirk_e`, )
- 이제 얼굴색을 좌/우 다르게 설정 할 수 있습니다. (`color_right_eyes`, `color_right_nose`, `color_right_mouth`, `color_right_side`)
- 그라데이션도 좌/우 다르게 설정 할 수 있도록 변경 (`my_protogen:gradation_right_points`)
- 사이드 패널 LED 순서를 반대로 사용 할 수 있는 설정 추가 (`hardware:side_panel:reversed`)
- 표정 셔플 기능이 추가되었습니다. 어떤 액팅을 할 지 고민될 때 사용해보세요!
- 빠른 표정 변경 기능에 프리셋이 추가되었습니다. 이제 여러 프리셋을 이용하여 더욱 편하게 사용할 수 있습니다.
- Uptime 표시가 추가되어 퍼슈팅을 한 시간을 편하게 확인 할 수 있습니다. 또한 DS3231 RTC를 연결하여 시간을 확인 할 수 있습니다.
- 투명도가 없는 흑백 동영상도 무지개빛으로 재생 할 수 있는 기능이 추가되었습니다.

### Feature
- You can play that dinosaur game. Jump using the Boop sensor! (Thank you for the idea, Yujin!!!)
- Supports Side Panel V2 with improved light diffusion.
- 5 basic emotions added. (default: `closed`, `smirk`, eyelash: `normal_blushing_e`, `closed_e`, `smirk_e`, )
- You can set the left/right face color differently. (`color_right_eyes`, `color_right_nose`, `color_right_mouth`, `color_right_side`)
- You can also set the the gradient to be set differently for left/right (`my_protogen:gradation_right_points`)
- Added a setting to reverse the order of the side panel LEDs (`hardware:side_panel:reversed`)
- Emotion shuffle function added. Use it when you are wondering what kind of acting to do!
- Presets added to the quick emotion change function. Now you can use it more conveniently with multiple presets.
- Uptime display added so you can easily check the time of the fursuit. You can also check the time by connecting the DS3231 RTC.
- Added a function to play black and white video without transparency in rainbow colors.

### 버그 수정
- IR 리모컨으로 사진 모드 동작하지 않는 문제 수정 (Fallen_Servoh 님 고맙습니다!)
- 동시 동작하는 스크립트간 시간 편차가 생기는 문제 수정
- 밝기 조절 메뉴에서 장갑형 컨트롤러로 빠져나갈 수 없는 문제 수정
- 밝기 조절 메뉴에서 시간 초과시 빠져나가지 않는 문제 수정
- 밝기 조절 메뉴에서 밝기 조절시 사이드 패널이 올바르지 않은 밝기로 조절되는 문제 수정
- 특정 조건의 gif 파일이 정상적으로 표시되지 않는 문제 수정
- 간헐적으로 프로그램이 멈추는 문제 해결

### Bugfix
- Fixed an issue where photo mode did not work with IR remote control (Thank you, Fallen_Servoh!)
- Fixed an issue where there was a time difference between concurrently running scripts
- Fixed an issue where the brightness control menu could not be exited with a glove controller
- Fixed an issue where the brightness control menu could not be exited when timed out
- Fixed an issue where the side panel was adjusted to an incorrect brightness when adjusting brightness in the brightness control menu
- Fixed an issue where gif files under certain conditions were not displayed properly
- Fixed an issue where the program would intermittently freeze

### 기타
- HUD 폰트 개선
- `Hub75Display::draw_image_newcolor` 코드 간결하게 수정
- 밝기 조절 메뉴 위치 이동
- ESP-Now 장갑 컨트롤러 프로토콜 수정

### Misc.
- Improved HUD font
- Simplified `Hub75Display::draw_image_newcolor` code
- Moved brightness control menu location
- Modified ESP-Now glove controller protocol


## 2024.5.2
### 주의 사항
- 표정을 저장하는 폴더 구조가 변경되었습니다. 이제 다른 사람에게 공유받은 표정을 더 편리하게 추가하세요! (`/emotions`)

### CAUTION
- The folder structure for storing emotion has changed. Now you can more conveniently add emotions shared by others! (`/emotions`)

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

### Feature
- Add support for GIF, MJPEG
- Add support for command in emotion (`video`, `video_loop`)
- HUD dashboard design change (change to icons instead of character)
- Add support dithering for HUD (`hardware:hud:dithering`)
- Adjusted eye size and improved framing for basic emotions
- Add emotions group with eyelashes (`eyelash`)
- Add 2 layers for special effect (`special`, `special2`)
- Add support for ESP-Now glove controller (Beta)
- Add emotion shortcut

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

### CAUTION
Storage format changed from SPIFFS to FFAT. You will need to re-upload not only the program but also the file system.

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
