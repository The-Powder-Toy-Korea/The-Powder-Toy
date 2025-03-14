#pragma once
#include "Config.h"
#include "SimulationConfig.h"
#include "common/String.h"

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
	if constexpr (LATENTHEAT)
	{
		sb << " LATENTHEAT";
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
	sb << "\bl\bU" << APPNAME << "\bU " << UPSTREAM_VERSION.displayVersion[0] << "." << UPSTREAM_VERSION.displayVersion[1] << " ko-KR_" << DISPLAY_VERSION[0] << "." << DISPLAY_VERSION[1] << " 한국어 - https://powdertoy.co.uk, irc.libera.chat #powder, https://tpt.io/discord\n"
	      "\n"
	      "\n"
	      "\bg<Ctrl+C>/<Ctrl+V>/<Ctrl+X>는 각각 복사, 붙여넣기, 잘라내기입니다.\n"
	      "\bg우측 탭에 마우스를 가져다 대면 해당 물질 탭에 있는 물질들이 하단 바에 표시됩니다.\n"
	      "\bg하단 바에서 물질 단추를 눌러 해당 물질을 선택합니다.\n"
	      "마우스로 아무 곳이나 드래그하여 자유형 곡선 형태로 물질을 그릴 수 있습니다.\n"
	      "<Shift> 키를 누른 상태로 드래그하여 직선 형태로 그릴 수 있습니다.\n"
	      "<Ctrl> 키를 누른 상태로 드래그하여 직사각형 형태로 그릴 수 있습니다.\n"
	      "<Ctrl+Shift>를 누른 상태에서 빈 공간을 클릭하면 해당 공간이 선택한 물질로 채워집니다.\n"
	      "마우스 휠을 돌리거나, <[>/<]> 키를 눌러 브러시의 크기를 조절할 수 있습니다.\n"
	      "<Alt> 키를 누른 상태로 클릭하거나 마우스 가운데 버튼으로 클릭하면 해당 물질을 \"스포이트\"할 수 있습니다.\n"
	      "<Ctrl+Z>로 실행 취소할 수 있습니다.\n"
	      "\n\bo<Z> 키를 눌러 일정 영역을 확대합니다. 그 상태로 클릭하면 확대 창이 고정됩니다. 휠을 사용하여 확대 수준을 조절합니다.\n"
	      "<Space> 키를 눌러 시뮬레이션을 일시 정지하고 재생할 수 있으며 <F> 키를 눌러 프레임 단위로 재생할 수 있습니다.\n"
	      "<S> 키를 눌러 세이브의 일부분을 '스탬프'로 저장합니다. <L> 키로 최근 스탬프를 불러오고, <K> 키로 목록을 엽니다.\n"
	      "<P> 키를 눌러 스크린샷을 저장할 수 있으며, 파일은 The Powder Toy 디렉터리에 바로 저장됩니다.\n"
	      "<H> 키를 눌러 HUD를 보이고 숨길 수 있으며, <D> 키를 눌러 HUD의 디버그 모드를 켜고 끌 수 있습니다.\n"
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
	sb << "\n\bt" << VersionInfo();
	return sb.Build();
}
