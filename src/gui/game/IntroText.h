#pragma once
#include "Config.h"
#include "SimulationConfig.h"
#include "common/String.h"
#include <cstring>

inline ByteString VersionInfo()
{
	ByteStringBuilder sb;
	sb << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1];
	if constexpr (!SNAPSHOT)
	{
		sb << "." << APP_VERSION.build;
	}
	sb << " " << IDENT;
	if constexpr (MOD)
	{
		sb << " MOD " << MOD_ID << " UPSTREAM " << UPSTREAM_VERSION.build;
	}
	if constexpr (SNAPSHOT)
	{
		sb << " SNAPSHOT " << APP_VERSION.build;
	}
	if constexpr (LUACONSOLE)
	{
		sb << " LUACONSOLE";
	}
	if constexpr (NOHTTP)
	{
		sb << " NOHTTP";
	}
	else if constexpr (ENFORCE_HTTPS)
	{
		sb << " HTTPS";
	}
	if constexpr (DEBUG)
	{
		sb << " DEBUG";
	}
	return sb.Build();
}

inline ByteString IntroText()
{
	ByteStringBuilder sb;
	sb << "\bl\bUThe Powder Toy\bU " << UPSTREAM_VERSION.displayVersion[0] << "." << UPSTREAM_VERSION.displayVersion[1] << " (ko-KR Build " << APP_VERSION.build << ") - https://powdertoy.co.uk, irc.libera.chat #powder, https://tpt.io/discord\n"
	      "\n"
	      "\n"
	      "\bg\bo<F1>\bg 키를 눌러 이 텍스트를 표시하거나 숨길 수 있습니다.\n"
	      "\n"
	      "\bg우측 탭에 마우스를 가져다 대면 해당 요소 범주의 요소들이 하단 바에 표시됩니다.\n"
	      "하단 바에서 \bo마우스 왼쪽/오른쪽\bg 클릭하여 해당 요소를 선택합니다.\n"
	      "마우스로 아무 곳이나 드래그하여 자유형 곡선 형태로 그릴 수 있습니다.\n"
	      "\n"
	      "\bo마우스 휠\bg을 돌리거나, \bo<[>\bg/\bo<]>\bg 키를 눌러 브러시의 크기를 조절할 수 있습니다. \bo<Tab>\bg 키를 눌러 브러시 모양을 변경합니다.\n"
	      "\bo마우스 가운데 버튼으로 클릭\bg하거나 \bo<Alt> 키를 누른 상태로 클릭\bg하여 해당 요소를 \"스포이트\"할 수 있습니다.\n"
	      "\bo<Ctrl+C>\bg/\bo<Ctrl+V>\bg/\bo<Ctrl+X>\bg는 각각 복사, 붙여넣기, 잘라내기입니다.\n"
	      "붙여 넣을 때, \bo<R>\bg 키를 눌러 회전하고, \bo<Shift+R>\bg 및 \bo<Shift+Ctrl+R>\bg로 수평 또는 수직으로 대칭 이동합니다.\n"
	      "\bo<Shift> 키를 누른 상태로 드래그\bg하여 직선을, \bo<Shift+Alt>를 누른 상태로 드래그\bg하여 수평/수직/대각선을 그립니다.\n"
	      "\bo<Ctrl> 키를 누른 상태로 드래그\bg하여 직사각형을, \bo<Ctrl+Alt>를 누른 상태로 드래그\bg하여 정사각형을 그립니다.\n\bo<Ctrl+Shift>를 누른 상태로 \bg빈 공간을 \bo클릭\bg하면 해당 공간이 선택한 요소로 채워집니다.\n"
	      "\n"
	      "\bo<Space>\bg 키를 눌러 시뮬레이션을 일시 정지/재생합니다. \bo<F>\bg 키로 한 프레임씩 재생하며, \bo<F5>\bg 키로 다시 불러옵니다.\n"
	      "\bo<Ctrl+Z>\bg로 실행 취소, \bo<Ctrl+Y>\bg 또는 \bo<Ctrl+Shift+Z>\bg로 다시 실행합니다.\n"
	      "\bo<S>\bg 키를 눌러 세이브의 일부분을 '스탬프'로 저장합니다. \bo<L>\bg 키로 최근 스탬프를 불러오고, \bo<K>\bg 키로 목록을 엽니다.\n"
		  "\n"
		  "\bo<0>-<9>\bg 키를 눌러 디스플레이 모드를 선택합니다.\n"
		  "\bo<H>\bg 키를 눌러 HUD를 보이고 숨길 수 있으며, \bo<D>\bg 키를 눌러 HUD의 디버그 모드를 켜고 끌 수 있습니다.\n"
	      "\bo<Z>\bg 키를 눌러 일정 영역을 확대합니다. 그 상태로 클릭하면 확대 창이 고정됩니다. 휠을 사용하여 확대 수준을 조절합니다.\n"
		  "\bo<Ctrl+F>\bg로 화면에서 선택한 요소를 강조 표시합니다.\n"
	      "\n";
	if constexpr (BETA)
	{
		sb << "\br이 버전은 베타 버전입니다. 세이브를 업로드하거나 이전 버전에서 작성된 로컬 세이브 및 스탬프를 열 수 없습니다.\n"
		      "\br세이브를 업로드하려면 정식 버전을 사용하십시오.\n";
	}
	else
	{
		sb << "\bg세이브의 업로드와 같은 기능을 사용하려면 \br" << SERVER << "/Register.html\bg에서 계정을 만드십시오.\n";
	}
	if constexpr (std::string_view(IDENT_PLATFORM) == "EMSCRIPTEN")
	{
		sb << "\brLocal saves and other data are managed by your browser and may be deleted unexpectedly. \bgIf in doubt, save online.\n";
	}
	sb << "\n\bt" << VersionInfo();
	return sb.Build();
}
