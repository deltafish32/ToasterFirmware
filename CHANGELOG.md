# Changelog


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
  - 모니터에 `!set video`를 입력해서 확인해보세요!
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
  - Enter `!set video` into monitor and check out!
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
