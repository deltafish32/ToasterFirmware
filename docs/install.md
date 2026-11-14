[한국어](install.md)
[English](install_en_US.md)

# 설치
1. [Visual Studio Code](https://code.visualstudio.com/Download) 설치 후 [PlatformIO](https://platformio.org/install/ide?install=vscode)를 설치합니다.
1. [Git](https://git-scm.com/downloads)을 설치합니다.
1. ESP32 보드를 연결하고, 좌측의 PlatformIO에서 `Upload Filesystem Image`를 먼저 해줍니다. 혹시나 `Monitoring` 상태라면 종료해주세요.
1. 하단의 `Upload` 버튼을 눌러 펌웨어를 설치합니다.

개발 보드가 CH340, CH9102 등의 USB 칩셋을 사용하는 경우, 드라이버를 설치하셔야 연결 가능합니다.

ESP32-S3는 USB CDC를 사용합니다. Windows 기준으로 기본 드라이버로도 어느정도 동작은 되지만, 정상적인 사용에 제한이 있으므로 [Zadig](https://zadig.akeo.ie/)를 이용하여 드라이버 설치 후 사용이 권장됩니다. Interface 0에 USB Serial(CDC) 드라이버를 설치하시면 됩니다.



# 설정
ToasterFirmware는 파일 시스템인 FFAT를 통해 데이터 파일을 Flash에 저장하여 사용합니다.
`Upload Filesystem Image`를 할 때 `/data` 폴더의 모든 파일이 업로드 됩니다.

그 중 `config.yaml` 파일에서 대부분의 설정이 관리됩니다. 기본적으로 yaml의 문법을 따르나, 모든 기능이 포함되어 있지 않으므로 최대한 형식을 맞춰주세요.

ESP32-S3를 사용하는 경우, `config_s3.yaml` 파일의 내용을 참고해주십시오. 파일 이름이나 내용을 변경하지 않고 업로드를 하게 되면 오류가 발생합니다.



# 나만의 프로토젠 만들기
## 색상
기본 포함된 표정 그림 파일을 수정하지 않고도 색상을 쉽게 변경 할 수 있습니다.

```yaml
my_protogen:
  color_eyes: '#00ff00'
  color_nose: color_eyes
  color_mouth: color_eyes
  color_side: '#0000ff'

  # ...
```

눈, 코, 입, 사이드 패널의 색상을 각각 지정 할 수 있으며, 예제와 같이 다른 부분의 색상과 동일하게 적용하는 것도 가능합니다.

HTML 색상 코드를 사용하며, 윈도우 그림판이나 [여기](https://www.w3schools.com/colors/colors_picker.asp)에서 확인 할 수 있습니다.


## 기본 표정
전원을 켰을 때 표시할 기본 표정을 지정합니다.

```yaml
my_protogen:
  # ...
  
  # 기본 표정
  eyes_default: eyes_normal
  nose_default: nose_default
  mouth_default: mouth_default
  side_default: side_default
```

표시할 요소는 `script` 폴더 안의 파일 이름을 사용하면 됩니다.



# 표정 만들기
표정은 png 이미지를 이용해 만들 수 있습니다. 눈/코/입 분리하여 만들어도 좋고, 1장의 그림으로 만들어도 좋습니다.

대신 전체 화면으로 여러 프레임을 만들 경우, 메모리 문제로 표시하지 못 할 수도 있습니다.


## 그림 준비
그림을 준비해주세요. 최대 64x32 픽셀, 8비트 RGBA PNG 형식으로 저장하면 됩니다.

준비된 그림을 `png` 폴더에 넣어주시면 됩니다.


## 스크립트
`script` 폴더에 각 스크립트를 저장하며, 각 파일은 아래와 같은 구조로 되어있습니다.

```yaml
version: 1
images:
  - /png/full/bsod.png
sequences:
  # draw, IMAGE_NO, COLOR, MODE, OFFSET_X, OFFSET_Y, TIME
  - draw, 0, original, mirror_only_offset, 12, 0, 0
```

- `version`: 스크립트 버전입니다. 추후 업그레이드를 위해 존재하며 1로 설정해주세요.
- `images`: 사용할 이미지 목록입니다. 파일 이름을 이용해 설정후 이미지 번호로 사용하며, 맨 첫 이미지는 0번 입니다.
- `sequences`: 순환할 이미지 목록입니다.
  - `FUNCTION`: 작업 목록입니다.
    - `draw`: 그림을 회전하지 않고 그립니다.
    - `draw_90`: 그림을 시계방향으로 90도 회전하여 그립니다.
    - `draw_180`: 그림을 180도 회전하여 그립니다.
    - `draw_270`: 그림을 시계방향으로 270도 회전하여 그립니다.
  - `IMAGE_NO`: 그릴 이미지 번호입니다. `images` 에서 불러온 이미지를 사용합니다. 맨 첫 이미지는 0번 입니다.
  - `COLOR`: 색상 모드입니다. 나의 프로토젠 색상 모드 사용시에만 적용됩니다.
    - `original`: 이미지의 원본 색상으로 표시합니다.
    - `eyes`: 눈 색상으로 표시합니다.
    - `nose`: 코 색상으로 표시합니다.
    - `mouth`: 입 색상으로 표시합니다.
    - `side`: 사이드 패널 색상으로 표시합니다.
  - `MODE`: 그림 모드입니다. 일반적인 용도로는 `mirror` 와 `mirror_only_offset`이 자주 사용됩니다.
    - `single`: 한 쪽에만 그립니다. 오른쪽 패널에 그리려면 X 오프셋 64부터 그리면 됩니다.
    - `copy`: 양쪽에 그대로 그립니다.
    - `mirror`: 양쪽에 대칭으로 그립니다. (얼굴 표정에 적합)
    - `mirror_only_offset`: 양쪽에 위치만 대칭으로 그립니다. (블루스크린 처럼 좌우 반전하지 않는 이미지 표시에 적합)
  - `OFFSET_X`: 그림 표시를 시작할 X 오프셋입니다. 왼쪽이 원점이며, 오른쪽으로 갈수록 증가합니다.
  - `OFFSET_Y`: 그림 표시를 시작할 Y 오프셋입니다. 위쪽이 원점이며, 아래쪽로 갈수록 증가합니다.
  - `TIME`: 해당 이미지를 표시할 시간이며 단위는 초 입니다. 시간이 초과되면 다음 줄의 이미지로 전환됩니다. 단일 이미지를 사용한다면 0으로 설정하면 됩니다.

스크립트를 직접 만들기 어려울수도 있으므로, 아래의 예시 중 비슷한 목적의 스크립트를 복사하여 사용하시는 것을 권장합니다.


### 예제 - 이미지 1장 표시
블루스크린을 표시합니다.

```yaml
version: 1
images:
  - /png/full/bsod.png
sequences:
  # draw, IMAGE_NO, COLOR, MODE, OFFSET_X, OFFSET_Y, TIME
  - draw, 0, original, mirror_only_offset, 12, 0, 0
```


### 예제 - 이미지 4장 왕복 표시
눈 깜빡임을 표시합니다. 0, 1, 2, 3, 2, 1 순서로 표시 후, 처음부터 순환합니다.

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


## 표정 등록
`config.yaml` 파일에서 표정을 등록합니다.

```yaml
emotions:
  # ...
  
  - name: my_emotion
    eyes: my_eye
    nose: my_nose
    mouth: my_mouth
    side: my_side
```

`eyes`, `nose`, `mouth`, `side`를 모두 등록해도 됩니다. 일부 부위만 등록해도 되며, 생략한 부위는 `default`의 요소를 사용합니다.



# 업로드
어떤 설정이든 변경하면 즉시 적용되지 않고, 파일을 파일시스템에 업로드해야 적용됩니다.

ESP32 보드를 연결하고, 좌측의 PlatformIO에서 `Upload Filesystem Image`를 먼저 해줍니다. 혹시나 `Monitoring` 상태라면 종료해주세요.

ESP32-S3에서는 [USB 저장소 모드](#usb-저장소-모드)로 파일을 직접 고칠 수도 있습니다.



# SD 카드
ESP32-S3의 경우 SD 카드를 사용할 수 있습니다. 훨씬 더 용량이 큰 동영상을 넣어서 재생할 수 있습니다.

내장 플래시와 SD 카드에 같은 이름의 표정이 있다면, SD 카드의 표정을 우선적으로 사용합니다.

만약 SD 카드가 인식되지 않는다면, 파일 시스템을 `FAT32`로, 할당 단위 크기를 `4096바이트`로 포맷 후 다시 시도해보십시오.

`exFAT`, `NTFS`로 포맷된 카드는 읽을 수 없습니다. 32GB보다 큰 카드는 Windows에서 기본으로 `exFAT`로 포맷되니 주의해주세요.



# USB 저장소 모드
> 주의: 베타 기능입니다. 중요한 파일은 PC에 따로 백업해두세요.

ESP32-S3의 경우, 내장 플래시(FFAT)를 USB 드라이브로 PC에 연결할 수 있습니다. `Upload Filesystem Image` 없이 탐색기에서 `config.yaml`이나 표정 파일을 바로 고칠 수 있습니다.

[SD 카드](#sd-카드)가 있다면 SD 카드도 별도의 드라이브로 함께 나타납니다.

## 빌드
USB 저장소 모드는 별도의 환경으로 빌드해야 합니다. PlatformIO 하단의 환경 선택에서 `env:adafruit_matrixportal_esp32s3_usbm`을 선택한 후 `Upload`를 해주세요.

이 환경은 USB를 기존(USB-CDC)과 다른 방식(TinyUSB)으로 사용합니다.
- 시리얼 모니터는 그대로 사용할 수 있지만, 업로드 후 COM 포트 번호가 바뀔 수 있습니다.
- 펌웨어가 재시작될 때마다 연결이 끊어지므로 부팅 초기 메시지는 확인하기 어렵습니다.
- 내장 USB JTAG 디버깅은 사용할 수 없습니다.
- 업로드가 되지 않으면 `BOOT` 버튼을 누른 채로 `RESET` 버튼을 눌러 다운로드 모드로 진입한 후 다시 시도해주세요.

## 사용법
1. HUD의 `Options` > `USB`에서 `ok`를 선택하거나, 시리얼 명령 `usb`를 입력합니다.
1. 모든 LED가 꺼지고 USB 저장소 모드로 재부팅됩니다.
1. PC에 USB로 연결하면 드라이브가 나타납니다. 파일을 복사하거나 수정합니다.
1. 작업이 끝나면 탐색기에서 **꺼내기**를 해주세요. 자동으로 재부팅하면서 변경 내용이 적용됩니다.
   - SD 카드도 함께 나타났다면 둘 중 하나만 꺼내도 됩니다. 다른 드라이브에 쓰는 중이라면 쓰기가 끝난 뒤 재부팅하며, 그동안 HUD에 `Waiting...`이 표시됩니다.

- HUD가 있다면 USB 저장소 모드 화면에 연결 상태와 읽기/쓰기 양(모든 드라이브의 합)이 표시됩니다.
- 꺼내기 없이 USB를 뽑아도 전원이 꺼지거나 3초 안에 재부팅되지만, PC가 아직 기록하지 못한 내용이 사라져 파일이 깨질 수 있습니다. 꼭 꺼내기를 해주세요.
- PC가 절전 모드로 들어가도 연결이 해제된 것으로 보고 재부팅합니다.
- USB를 연결하지 않은 채 USB 저장소 모드에 들어갔다면, 전원을 껐다 켜면 원래대로 부팅합니다.
- 플래시 특성상 쓰기 속도가 느립니다. 특히 작은 파일이 많은 폴더는 복사에 시간이 오래 걸릴 수 있습니다.
- 파일이 깨져 읽기 오류가 나는 경우:
  - `Upload Filesystem Image`로 다시 써주세요.
  - 또는 파일 시스템 `FAT`, 할당 단위 크기 `4096 바이트`로 포맷 후 데이터를 다시 넣어주어도 됩니다.
- SD 카드:
  - USB 저장소 모드에 들어가기 전에 꽂혀 있던 카드만 나타납니다. USB 저장소 모드 중에는 카드를 빼거나 꽂지 마세요.
  - 읽을 수 없는 카드도 드라이브로 나타나므로 PC에서 포맷할 수 있습니다. 이때 파일 시스템은 반드시 `FAT32`를 선택해주세요.
  - SD 카드를 초기화하지 못하면 HUD에 `SD card error`가 표시되고, 드라이브는 비어 있는 상태로 나타납니다.
  - USB로는 속도가 느리므로, 큰 파일은 카드를 빼서 PC의 카드 리더로 옮기는 것이 빠릅니다.

## 설정 오류 시 자동 진입
USB 저장소 모드 빌드에서는 `config.yaml` 오류 등으로 부팅에 실패하면 USB 저장소 모드로 자동 진입합니다. PC에 연결해 파일을 고친 후 꺼내기를 하면 다시 부팅합니다.



# 디버그 모드
> 주의: 이 기능은 언제든지 변경 될 수 있습니다!

ESP32 보드에 USB로 연결된 상태라면 PC로도 조작이 가능합니다. 컨트롤러가 준비되지 않았거나, 디버깅 목적으로 유용하게 활용 할 수 있습니다.

Visual Studio Code 화면 하단의 `Platformio: Serial Monitor` 버튼을 누르시면 모니터링 모드로 진입됩니다. 이 화면에서 키를 누르시면 조작이 가능합니다.


## 표정 전환
| 키 | 기능 | 비고 |
| - | - | - |
| 0 | 화면 끄기 | |
| 1 | 무표정 (normal) | |
| 2 | 날카로운 표정 (badass) | |
| 3 | 웃는 표정 (grin) | |
| 4 | 우려하는 표정 (worry) | |
| 5 | 하트 눈 (love) | |
| 6 | 어지러운 표정 (confusion) | |
| 7 | 물음표정 (qmark) | |
| 8 | 귀여운 표정 (unyuu) | |
| 9 | 콩눈 (bean) | |
| P | 블루스크린 (bsod) | |
| W | 백색 화면 (white) | 최대 밝기에서 사용시, 전력 문제로 정상적으로 표시되지 않을 수 있음 |
| O | 음악 시각화 에뮬레이션 모드 (파티 모드) (festive) | |
| ' | 로딩 중 (loading) | |


## 장갑 컨트롤러 에뮬레이션
대소문자에 따라 동작이 구분됩니다.
- 소문자: 짧게 누르기
- 대문자: 길게 누르기

| 키 | 기능 | 비고 |
| - | - | - |
| F | 왼손 검지 |  |
| D | 왼손 중지 |  |
| S | 왼손 약지 |  |
| A | 왼손 소지 | 미사용, 예약됨 |
| J | 오른손 검지 |  |
| K | 오른손 중지 |  |
| L | 오른손 약지 |  |
| ;(:) | 오른손 소지 | 미사용, 예약됨 |


## 특수
| 키 | 기능 | 비고 |
| - | - | - |
| Z | 색상 모드 - 원색 (original) | |
| X | 색상 모드 - 나만의 프로토젠 (personal) | |
| C | 색상 모드 - 무지개(단색) (rainbow_single) | |
| V | 색상 모드 - 무지개 (rainbow) | |
| B | 색상 모드 - 그라데이션 (gradation) | |
| E | 사진 모드 토글 | 화면의 변화를 멈추는 모드 |
| 왼쪽 방향키 | 왼쪽 | 메뉴에서 사용 (뒤로가기) |
| 아래쪽 방향키  | 아래쪽 | 메뉴에서 사용 |
| 위쪽 방향키 | 위쪽 | 메뉴에서 사용 |
| 오른쪽 방향키 | 오른쪽 | 메뉴에서 사용 (선택) |
| M | Boop 센서 디버깅 | 시리얼 통신으로 Boop 센서 측정 값(mm) 표시 기능 토글 |
| \ | HUD 디더링 토글 | |
| TAB | 다음 표정으로 변경 | 등록된 순서 내의 다음 표정으로 빠르게 변경 |


## 명령어
`Enter` 키 혹은 `!` 키를 누른 후 명령어를 입력하면 됩니다.

| Command | Function | Note |
| - | - | - |
| `help`                 | 사용 가능한 명령어를 모두 표시합니다. | |
| `reset`, `restart` or `reboot` | Soft reset | |
| `b`                    | 밝기를 해당 값(0-100)으로 설정합니다. 조도센서 사용 중에는 자동 밝기가 일시 중지되며, `b auto`로 다시 켤 수 있습니다. | 예제: `b 100`, `b auto` |
| `ls` or `emotions`     | 로딩된 모든 표정을 표시합니다. | |
| `set`                  | 해당 표정으로 변경합니다. | 예제: `set normal` |
| `mac`                  | mac 주소를 표시합니다. | |
| `boop`                 | Boop 센서를 활성화합니다. | |
| `noboop`               | Boop 센서를 비활성화합니다. | |
| `dithering`            | HUD dithering을 활성화합니다. | |
| `nodithering`          | HUD dithering을 비활성화합니다. | |
| `datetime`             | 내장 시계의 날짜와 시간을 표시합니다. | |
| `settime`              | 내장 시계의 시간을 설정합니다. | 예제: `settime 12:34` 또는 `settime 12:34:56` |
| `setdatetime`          | 내장 시계의 날짜와 시간을 설정합니다. | 예제: `setdatetime 2023-12-08 12:34:56` |
| `color`                | 색상 모드를 변경합니다. | 예제: `color personal2` |
| `usb`                  | USB 저장소 모드로 재부팅합니다. | [USB 저장소 모드](#usb-저장소-모드) 빌드에서만 사용 가능 |
| `loglevel`             | 로그 레벨을 표시하거나 변경합니다. | 예제: `loglevel`, `loglevel info`, `loglevel 3`. [로그 레벨](#로그-레벨) 참고 |


## 로그 레벨
시리얼 모니터에 출력할 로그의 양을 정할 수 있습니다. 설정한 레벨과 그보다 중요한 로그만 출력됩니다.

| 레벨 | 숫자 | 출력되는 로그 |
| - | - | - |
| `none`    | 0 | 출력하지 않음 |
| `error`   | 1 | 오류 |
| `warn`    | 2 | 오류, 경고 |
| `info`    | 3 | 오류, 경고, 정보 |
| `debug`   | 4 | 오류, 경고, 정보, 디버그 (기본값) |
| `verbose` | 5 | 모든 로그 |

부팅 시의 레벨은 `config.yaml`의 `log_level`로 정합니다. 설정을 읽기 전의 부팅 로그는 기본값(`debug`)으로 출력됩니다.
```yaml
log_level: info
```

실행 중에는 `loglevel` 명령으로 바꿀 수 있으며, 재부팅하면 `config.yaml`의 값으로 돌아갑니다.

빌드 플래그 `TF_LOG_MAX_LEVEL`보다 높은 레벨의 로그는 펌웨어에 포함되지 않으므로, 레벨을 올려도 출력되지 않습니다. 기본값은 `4`(`debug`)이며, `verbose` 로그를 보려면 `platformio.ini`의 `build_flags`에 다음을 추가한 후 다시 업로드해주십시오.
```ini
  -DTF_LOG_MAX_LEVEL=5
```
