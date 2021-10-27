#pragma once
#include "Config.h"

const char *const introTextData =
	"\blThe Powder Toy " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " ko-KR_" MTOS(MOD_MAJOR_VERSION) "." MTOS(MOD_MINOR_VERSION) " 한국어 - https://powdertoy.co.uk, irc.libera.chat #powder\n"
	"\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\xEE\x81\xA9\n"
	"\n"
	"\bg[Ctrl] + [C]/[V]/[X]는 각각 복사, 붙여넣기, 잘라내기입니다.\n"
	"\bg우측 탭에 마우스를 가져다 대면 해당 물질 탭에 있는 물질들이 하단 바에 표시됩니다.\n"
	"\bg하단 바에서 물질 단추를 클릭하면 해당 물질이 선택됩니다.\n"
	"마우스로 아무 곳이나 드래그하여 자유형 곡선 형태로 물질을 그릴 수 있습니다.\n"
	"[Shift]를 누른 상태로 드래그하면 직선 형태를 그려지며, [Ctrl]를 누른 상태로 드래그하면 직사각형 형태로 그려집니다.\n"
	"[Ctrl] + [Shift]를 누를 상태에서 빈 공간을 클릭하면 해당 공간이 선택한 물질로 채워집니다.\n"
	"마우스 휠을 돌리거나, [[]/[]]를 눌러 브러시의 크기를 조정할 수 있습니다.\n"
	"[Alt]를 누른 상태로 클릭하거나 마우스 가운데 버튼으로 클릭하면 해당 물질을 \"스포이트\"할 수 있습니다.\n"
	"[Ctrl] + [Z]를 눌러 실행 취소를 할 수 있습니다.\n"
	"\n\bo[Z]를 눌러 일정 영역을 확대하여 볼 수 있습니다. 그 상태로 클릭하면 움직이는 확대 창이 고정됩니다.\n"
	"해당 창 안에서 휠을 사용하여 확대 수준을 조정할 수 있습니다.\n"
	"스페이스 바를 사용하여 시뮬레이션의 일시 정지를 전환할 수 있으며 [F]를 눌러 프레임 단위로 재생할 수 있습니다.\n"
	"[S]를 눌러 시뮬레이션의 일부분을 '스탬프'로 저장할 수 있습니다.\n"
	"[L]를 눌러 가장 최근에 저장한 스탬프를 불러올 수 있으며, [K]를 눌러 현재까지 저장한 모든 스탬프를 확인할 수 있습니다.\n"
	"[P]를 눌러 스크린샷을 저장할 수 있으며, 파일은 The Powder Toy 디렉토리에 바로 저장됩니다.\n"
	"[H]를 눌러 HUD 표시를 전환할 수 있으며, [D]를 눌러 HUD의 디버그 모드를 전환할 수 있습니다.\n"
	"\n"
	"기여자: \bg스타니슬라프 K. 스코로넥 (오리지널 The Powder Toy를 디자인함),\n"
	"\bg사이먼 로버트쇼, 스크레자노프 사블리, cracker64, Catelite, 브라이언 호일, 네이선 커즌스, jacksonmj,\n"
	"\bg펠릭스 월린, 리우베 모쉬, 앤서니 부트, Me4502, MaksProg, jacob1, mniip, LBPHacker\n"
	"\bo번역자: \bg피어(The Xphere), 파토매니아, Hopz, LegenDUST - The Powder Toy 공식 네이버 카페\n"
	"\n"
#ifndef BETA
	"\bg시뮬레이션의 업로드와 같은 기능을 사용하려면 \brhttps://powdertoy.co.uk/Register.html\bg에서 계정을 만드세요.\n"
#else
	"\br이 버전은 베타 버전이며, 시뮬레이션을 업로드할 수 없습니다. 시뮬레이션을 업로드하려면 정식 버전을 사용하세요.\n"
#endif
	"\n"
	"\bt" MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) "." MTOS(BUILD_NUM) " " IDENT_PLATFORM
#ifdef SNAPSHOT
	" SNAPSHOT " MTOS(SNAPSHOT_ID)
#elif MOD_ID > 0
	" MODVER " MTOS(SNAPSHOT_ID)
#endif
#ifdef X86
	" X86"
#endif
#ifdef X86_SSE
	" X86_SSE"
#endif
#ifdef X86_SSE2
	" X86_SSE2"
#endif
#ifdef X86_SSE3
	" X86_SSE3"
#endif
#ifdef LUACONSOLE
	" LUACONSOLE"
#endif
#ifdef GRAVFFT
	" GRAVFFT"
#endif
#ifdef REALISTIC
	" REALISTIC"
#endif
	;
