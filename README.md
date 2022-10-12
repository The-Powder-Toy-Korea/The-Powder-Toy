The Powder Toy - 2021년 7월
==========================

[The Powder Toy 공식 홈페이지](https://powdertoy.co.uk/Download.html)에서 최신 버전의 오리지널 TPT를 내려받으세요.

시뮬레이션의 업로드와 같은 기능을 사용하려면 [계정을 만드세요](https://powdertoy.co.uk/Register.html).
The Powder Toy 공식 포럼은 [여기](https://powdertoy.co.uk/Discussions/Categories/Index.html)에 있습니다.

무언가를 터뜨려 보고 싶은 적이 있나요? 또는 원자력 발전소를 직접 운영해 보고 싶은 적은 있었나요? 자신만의 CPU를 구현해 보고 싶나요? The Powder Toy는 이 모든 것들을 직접 할 수 있게 만들어줍니다. 물론 이뿐만이 아닌 더 많은 것들을요!

The Powder Toy는 무료 물리학 샌드박스 게임으로서, 공기 압력 및 흐름, 열, 중력, 그리고 셀 수 없이 많은 물질들의 상호작용을 시뮬레이션합니다! 또한 복잡한 기계, 총, 폭탄, 현실적인 지형 및 거의 모든 것을 구성하는 데 사용할 수 있는 다양한 건축 자재, 액체, 가스 및 전자 부품도 모두 제공됩니다. 그리고 제공된 모든 것들을 활용하여 멋진 폭발을 구경하고, 복잡한 배선을 추가하고, 작은 스틱맨과 놀거나, 기계를 작동시킬 수 있습니다. 커뮤니티에서 만든 수천 개의 다른 저장 항목을 탐색하고 재생하거나 직접 업로드할 수 있습니다. 여러분의 창작물을 환영합니다!

Lua API가 있어 작업을 자동화하거나 게임용 플러그인을 만들 수도 있습니다. The Powder Toy는 무료이며 소스 코드는 GNU 일반 공중 사용 허가서에 따라 배포되므로 직접 게임을 수정하거나 개발을 도울 수 있습니다.

빌드 안내
===========================================================================

TPT를 빌드하려면 [The Powder Toy 공식 위키](https://powdertoy.co.uk/Wiki/W/Main_Page/ko.html)의 _개발 안내_ 문단을 참조하세요.

기여자
===========================================================================

* 스타니슬라프 K. 스코브로네크 (오리지널 The Powder Toy를 디자인함)
* 사이먼 로버트쇼
* 스크레사노우 사벨리
* cracker64
* Catelite
* 브라이언 호일
* 네이선 커즌스
* jacksonmj
* 펠릭스 월린
* 리우어 모스
* 앤서니 부트
* Me4502
* MaksProg
* jacob1
* mniip
* LBPHacker

번역자
===========================================================================

* 피어 (The Xphere, xphere07)
* 뭐지What (NeuroWhAI)
* 파토매니아
* Hopz
* LegenDUST

사용된 라이브러리 및 애셋
===========================================================================

* [BSON](https://github.com/interactive-matter/bson-c)
* [bzip2](http://www.bzip.org/)
* [FFTW](http://fftw.org/)
* [JsonCpp](https://github.com/open-source-parsers/jsoncpp)
* [libcurl](https://curl.se/libcurl/)
* [Lua](https://www.lua.org/)
* [LuaJIT](https://luajit.org/)
* [LuaSocket](http://w3.impa.br/~diego/software/luasocket/)
* [Mallangche](https://github.com/JammPark/Mallangche)
* [mbedtls](https://www.trustedfirmware.org/projects/mbed-tls/)
* [SDL](https://libsdl.org/)
* [zlib](https://www.zlib.net/)

사용법
===========================================================================

물질 단추를 클릭하여 그림판과 같이 그리기 공간에 그립니다. 이제 마음껏 시뮬레이션을 진행할 수 있습니다.

단축키
===========================================================================

| 단축키                  | 동작                                                            |
| ----------------------- | --------------------------------------------------------------- |
| TAB                     | 브러시 모양 변경 (원, 사각형, 삼각형)                              |
| Space                   | 일시 정지                                                        |
| Q / Esc                 | 나가기                                                           |
| Z                       | 선택 영역 확대                                                   |
| S                       | 스탬프 저장, STK2이 있을 때에는 \[Ctrl\]를 누른 상태로 사용합니다.  |
| L                       | 마지막으로 저장된 스탬프 불러오기                                  |
| K                       | 스탬프 목록                                                      |
| 0-9                     | 보기 모드 설정                                                    |
| P / F2                  | 스크린샷을 PNG로 저장                                             |
| E                       | 물질 검색                                                        |
| F                       | 다음 프레임                                                      |
| G                       | 격자 크기 증가                                                   |
| Shift + G               | 격자 크기 감소                                                   |
| H                       | HUD 표시 전환                                                    |
| Ctrl + H / F1           | 시작 텍스트 표시                                                  |
| D / F3                  | 디버그 모드, STK2이 있을 때에는 \[Ctrl\]를 누른 상태로 사용합니다.    |
| I                       | 압력 및 공기 흐름 뒤집기                                            |
| W                       | 중력 모드 전환, STK2이 있을 때에는 \[Ctrl\]를 누른 상태로 사용합니다. |
| Y                       | 공기 모드 전환                                                     |
| B                       | 도색 편집 메뉴                                                   |
| Ctrl + B                | 도색 효과 표시 전환                                              |
| N                       | 뉴턴 중력 전환                                                   |
| U                       | 복사열 전환                                                      |
| Ctrl + I                | The Powder Toy 설치, 세이브 및 스탬프를 더블클릭으로 불러오기 위해 필요 |
| Backtick                | 콘솔 열기                                                        |
| =                       | 압력 및 공기 흐름 초기화                                          |
| Ctrl + =                | 모든 전류 제거                                                   |
| \[                      | 브러시 크기 줄이기                                               |
| \]                      | 브러시 크기 키우기                                               |
| Alt + \[                | 브러시 크기를 단일 픽셀 단위로 줄이기                              |
| Alt + \]                | 브러시 크기를 단일 픽셀 단위로 키우기                              |
| Ctrl + C / V / X        | 복사 · 붙여넣기 · 잘라내기                                        |
| Ctrl + Z                | 실행 취소                                                        |
| Ctrl + Y                | 다시 실행                                                        |
| Ctrl + 드래그            | 직사각형 그리기                                                   |
| Shift + 드래그           | 선 그리기                                                        |
| 가운데 클릭              | 물질 스포이트                                                    |
| Alt + 왼쪽 클릭          | 물질 스포이트                                                    |
| 스크롤                   | 브러시 크기 변경                                                 |
| Ctrl + 스크롤            | 브러시 높이 변경                                                 |
| Shift + 스크롤           | 브러시 너비 변경                                                 |
| Shift + R               | 붙여넣을 때 선택 영역 수평 뒤집기                                  |
| Ctrl + Shift + R        | 붙여넣을 때 선택 영역 수직 뒤집기                                  |
| R                       | 붙여넣을 때 선택 영역 회전                                        |

명령줄 명령
---------------------------------------------------------------------------

| 명령                  | 설명                                              | 예시                              |
| --------------------- | ------------------------------------------------ | --------------------------------- |
| `scale:SIZE`          | 창 스케일 조정                                    | `scale:2`                         |
| `kiosk`               | 전체 화면                                         |                                   |
| `proxy:SERVER[:PORT]` | 사용할 프록시 서버                                 | `proxy:wwwcache.lancs.ac.uk:8080` |
| `open FILE`           | 세이브 및 스탬프 열기                              |                                   |
| `ddir DIRECTORY`      | 설정 및 스탬프가 저장된 디렉터리                    |                                   |
| `ptsave:SAVEID`       | 온라인 세이브 열기, URL이 ptsave에서 사용됨         | `ptsave:2198`                     |
| `disable-network`     | 인터넷 연결 비활성화                               |                                   |
| `redirect`            | 출력을 stdout.txt / stderr.txt에 리다이렉트        |                                   |
| `cafile:CAFILE`       | 인증서 번들 경로 설정                                | `cafile:/etc/ssl/certs/ca-certificates.crt` |
| `capath:CAPATH`       | 인증서 디렉터리 경로 설정                             | `capath:/etc/ssl/certs`           |
