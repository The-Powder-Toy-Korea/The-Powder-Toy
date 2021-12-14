#include "OptionsView.h"

#include <cstdio>
#include <cstring>
#include "SDLCompat.h"

#include "OptionsController.h"
#include "OptionsModel.h"

#include "client/Client.h"
#include "common/Platform.h"
#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "simulation/ElementDefs.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/DropDown.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"

OptionsView::OptionsView():
	ui::Window(ui::Point(-1, -1), ui::Point(320, 340))
	{

	auto autowidth = [this](ui::Component *c) {
		c->Size.X = Size.X - c->Position.X - 12;
	};
	
	ui::Label * tempLabel = new ui::Label(ui::Point(4, 1), ui::Point(Size.X-8, 22), "시뮬레이션 설정");
	tempLabel->SetTextColour(style::Colour::InformationTitle);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	autowidth(tempLabel);
	AddComponent(tempLabel);

	class Separator : public ui::Component
	{
		public:
		Separator(ui::Point position, ui::Point size) : Component(position, size){}
		virtual ~Separator(){}

		void Draw(const ui::Point& screenPos) override
		{
			GetGraphics()->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 180);
		}		
	};
	
	Separator *tmpSeparator = new Separator(ui::Point(0, 22), ui::Point(Size.X, 1));
	AddComponent(tmpSeparator);

	int currentY = 6;
	scrollPanel = new ui::ScrollPanel(ui::Point(1, 23), ui::Point(Size.X-2, Size.Y-39));
	
	AddComponent(scrollPanel);

	heatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "열 시뮬레이션 \bg버전 34 이상", "");
	autowidth(heatSimulation);
	heatSimulation->SetActionCallback({ [this] { c->SetHeatSimulation(heatSimulation->GetChecked()); } });
	scrollPanel->AddChild(heatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bg비활성화 시 정상적이지 않은 동작을 유발할 수 있음");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	ambientHeatSimulation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "복사열 시뮬레이션 \bg버전 50 이상", "");
	autowidth(ambientHeatSimulation);
	ambientHeatSimulation->SetActionCallback({ [this] { c->SetAmbientHeatSimulation(ambientHeatSimulation->GetChecked()); } });
	scrollPanel->AddChild(ambientHeatSimulation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bg활성화 시 일부 세이브와 충돌을 일으킬 수 있음");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	newtonianGravity = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "뉴턴 중력 시뮬레이션 \bg버전 48 이상", "");
	autowidth(newtonianGravity);
	newtonianGravity->SetActionCallback({ [this] { c->SetNewtonianGravity(newtonianGravity->GetChecked()); } });
	scrollPanel->AddChild(newtonianGravity);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bg일정 성능 이하의 컴퓨터에서 성능 저하를 일으킬 수 있음");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=16;
	waterEqualisation = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "액체 높이 균등화 \bg버전 61 이상", "");
	autowidth(waterEqualisation);
	waterEqualisation->SetActionCallback({ [this] { c->SetWaterEqualisation(waterEqualisation->GetChecked()); } });
	scrollPanel->AddChild(waterEqualisation);
	currentY+=14;
	tempLabel = new ui::Label(ui::Point(24, currentY), ui::Point(1, 16), "\bg다량의 액체가 있을 시 성능 저하를 일으킬 수 있음");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=19;
	airMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(airMode);
	airMode->AddOption(std::pair<String, int>("켜기", 0));
	airMode->AddOption(std::pair<String, int>("압력 끄기", 1));
	airMode->AddOption(std::pair<String, int>("바람 끄기", 2));
	airMode->AddOption(std::pair<String, int>("끄기", 3));
	airMode->AddOption(std::pair<String, int>("고정", 4));
	airMode->SetActionCallback({ [this] { c->SetAirMode(airMode->GetOption().second); } });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "공기 시뮬레이션 모드");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	ambientAirTemp = new ui::Textbox(ui::Point(Size.X-95, currentY), ui::Point(60, 16));
	ambientAirTemp->SetActionCallback({ [this] {
		UpdateAirTemp(ambientAirTemp->GetText(), false);
	} });
	ambientAirTemp->SetDefocusCallback({ [this] {
		UpdateAirTemp(ambientAirTemp->GetText(), true);
	}});
	scrollPanel->AddChild(ambientAirTemp);

	ambientAirTempPreview = new ui::Button(ui::Point(Size.X-31, currentY), ui::Point(16, 16), "", "Preview");
	scrollPanel->AddChild(ambientAirTempPreview);

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "기본 복사열 온도");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	gravityMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(gravityMode);
	gravityMode->AddOption(std::pair<String, int>("수직", 0));
	gravityMode->AddOption(std::pair<String, int>("끄기", 1));
	gravityMode->AddOption(std::pair<String, int>("중심", 2));
	gravityMode->SetActionCallback({ [this] { c->SetGravityMode(gravityMode->GetOption().second); } });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "중력 방향");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	edgeMode = new ui::DropDown(ui::Point(Size.X-95, currentY), ui::Point(80, 16));
	scrollPanel->AddChild(edgeMode);
	edgeMode->AddOption(std::pair<String, int>("공허", 0));
	edgeMode->AddOption(std::pair<String, int>("벽", 1));
	edgeMode->AddOption(std::pair<String, int>("반복", 2));
	edgeMode->SetActionCallback({ [this] { c->SetEdgeMode(edgeMode->GetOption().second); } });

	tempLabel = new ui::Label(ui::Point(8, currentY), ui::Point(Size.X-96, 16), "모서리");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	tmpSeparator = new Separator(ui::Point(0, currentY), ui::Point(Size.X, 1));
	scrollPanel->AddChild(tmpSeparator);

	currentY+=4;
	scale = new ui::DropDown(ui::Point(8, currentY), ui::Point(40, 16));
	{
		int current_scale = ui::Engine::Ref().GetScale();
		int ix_scale = 1;
		bool current_scale_valid = false;
		do
		{
			if (current_scale == ix_scale)
				current_scale_valid = true;
			scale->AddOption(std::pair<String, int>(String::Build(ix_scale), ix_scale));
			ix_scale += 1;
		}
		while (ui::Engine::Ref().GetMaxWidth() >= ui::Engine::Ref().GetWidth() * ix_scale && ui::Engine::Ref().GetMaxHeight() >= ui::Engine::Ref().GetHeight() * ix_scale);
		if (!current_scale_valid)
			scale->AddOption(std::pair<String, int>("현재", current_scale));
	}
	scale->SetActionCallback({ [this] { c->SetScale(scale->GetOption().second); } });
	scrollPanel->AddChild(scale);

	tempLabel = new ui::Label(ui::Point(scale->Position.X+scale->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- 대화면 디스플레이용 창 스케일 조정");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	resizable = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "창 크기 조절", "");
	autowidth(resizable);
	resizable->SetActionCallback({ [this] { c->SetResizable(resizable->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(resizable->Position.X+Graphics::textwidth(resizable->GetText())+20, currentY), ui::Point(1, 16), "\bg- 창 크기를 조절하거나 최대화할 수 있도록\n허용합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(resizable);

	currentY+=20;
	fullscreen = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "전체 화면", "");
	autowidth(fullscreen);
	fullscreen->SetActionCallback({ [this] { c->SetFullscreen(fullscreen->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(fullscreen->Position.X+Graphics::textwidth(fullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- 전체 화면으로 플레이합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fullscreen);

	currentY+=20;
	altFullscreen = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "해상도 변경", "");
	autowidth(altFullscreen);
	altFullscreen->SetActionCallback({ [this] { c->SetAltFullscreen(altFullscreen->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(altFullscreen->GetText())+20, currentY), ui::Point(1, 16), "\bg- 최적화된 해상도로 조정합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(altFullscreen);

	currentY+=20;
	forceIntegerScaling = new ui::Checkbox(ui::Point(23, currentY), ui::Point(1, 16), "강제 정수 스케일링", "");
	autowidth(forceIntegerScaling);
	forceIntegerScaling->SetActionCallback({ [this] { c->SetForceIntegerScaling(forceIntegerScaling->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(altFullscreen->Position.X+Graphics::textwidth(forceIntegerScaling->GetText())+20, currentY), ui::Point(1, 16), "\bg- 픽셀이 선명해지도록 조정합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(forceIntegerScaling);

	currentY+=20;
	fastquit = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "빠른 종료", "");
	autowidth(fastquit);
	fastquit->SetActionCallback({ [this] { c->SetFastQuit(fastquit->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(fastquit->Position.X+Graphics::textwidth(fastquit->GetText())+20, currentY), ui::Point(1, 16), "\bg- 닫기 단추를 누르면 바로 종료합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(fastquit);

	currentY+=20;
	showAvatars = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "프로필 사진 표시", "");
	autowidth(showAvatars);
	showAvatars->SetActionCallback({ [this] { c->SetShowAvatars(showAvatars->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(showAvatars->Position.X+Graphics::textwidth(showAvatars->GetText())+20, currentY), ui::Point(1, 16), "\bg- 네트워크 속도가 느리다면\n비활성화하는 것을 권장합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(showAvatars);

	currentY += 20;
	momentumScroll = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "부드러운 스크롤 활성화", "");
	autowidth(momentumScroll);
	momentumScroll->SetActionCallback({ [this] { c->SetMomentumScroll(momentumScroll->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(momentumScroll->Position.X + Graphics::textwidth(momentumScroll->GetText()) + 20, currentY), ui::Point(1, 16), "\bg- 부드러운 스크롤을 활성화합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(momentumScroll);

	currentY+=20;
	mouseClickRequired = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "카테고리 고정", "");
	autowidth(mouseClickRequired);
	mouseClickRequired->SetActionCallback({ [this] { c->SetMouseClickrequired(mouseClickRequired->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(mouseClickRequired->Position.X+Graphics::textwidth(mouseClickRequired->GetText())+20, currentY), ui::Point(1, 16), "\bg- 우측 물질 카테고리를 클릭으로 전환합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(mouseClickRequired);

	currentY+=20;
	includePressure = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "저장 시 압력 포함", "");
	autowidth(includePressure);
	includePressure->SetActionCallback({ [this] { c->SetIncludePressure(includePressure->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(includePressure->Position.X+Graphics::textwidth(includePressure->GetText())+20, currentY), ui::Point(1, 16), "\bg- 복사하거나 저장할 때 압력을 포함합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(includePressure);

	currentY+=20;
	perfectCirclePressure = new ui::Checkbox(ui::Point(8, currentY), ui::Point(1, 16), "정확한 원 브러시", "");
	autowidth(perfectCirclePressure);
	perfectCirclePressure->SetActionCallback({ [this] { c->SetPerfectCircle(perfectCirclePressure->GetChecked()); } });
	tempLabel = new ui::Label(ui::Point(perfectCirclePressure->Position.X+Graphics::textwidth(perfectCirclePressure->GetText())+20, currentY), ui::Point(1, 16), "\bg- 브러시의 원을 정확하게 조정합니다.");
	autowidth(tempLabel);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);
	scrollPanel->AddChild(perfectCirclePressure);

	currentY+=20;
	decoSpace = new ui::DropDown(ui::Point(8, currentY), ui::Point(60, 16));
	decoSpace->SetActionCallback({ [this] { c->SetDecoSpace(decoSpace->GetOption().second); } });
	scrollPanel->AddChild(decoSpace);
	decoSpace->AddOption(std::pair<String, int>("sRGB", 0));
	decoSpace->AddOption(std::pair<String, int>("Linear", 1));
	decoSpace->AddOption(std::pair<String, int>("Gamma 2.2", 2));
	decoSpace->AddOption(std::pair<String, int>("Gamma 1.8", 3));

	tempLabel = new ui::Label(ui::Point(decoSpace->Position.X+decoSpace->Size.X+3, currentY), ui::Point(Size.X-40, 16), "\bg- 도색 도구에서 사용할 색 공간");
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	scrollPanel->AddChild(tempLabel);

	currentY+=20;
	ui::Button * dataFolderButton = new ui::Button(ui::Point(8, currentY), ui::Point(90, 16), "데이터 폴더 열기");
	dataFolderButton->SetActionCallback({ [] {
		ByteString cwd = Platform::GetCwd();
		if (!cwd.empty())
			Platform::OpenURI(cwd);
		else
			fprintf(stderr, "cannot open data folder: getcwd(...) failed\n");
	} });
	scrollPanel->AddChild(dataFolderButton);

	ui::Button * migrationButton = new ui::Button(ui::Point(Size.X - 178, currentY), ui::Point(163, 16), "공유 데이터 디렉터리로 이동");
	migrationButton->SetActionCallback({ [] {
		ByteString from = Platform::originalCwd;
		ByteString to = Platform::sharedCwd;
		new ConfirmPrompt("디렉터리를 이동할까요?", "이것은\n\bt" + from.FromUtf8() + "\bw\n에 있는 모든 스탬프, 세이브, 스크립트를 디렉터리\n\bt" + to.FromUtf8() + "\bw로 이동합니다.\n\n" +
			 "이미 존재하는 파일은 덮어씌여지지 않을 것입니다.", { [=] () {
				 String ret = Platform::DoMigration(from, to);
				new InformationMessage("이동됨", ret, false);
			 } });
	} });
	scrollPanel->AddChild(migrationButton);

	ui::Button * tempButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "확인");
	tempButton->SetActionCallback({ [this] { c->Exit(); } });
	AddComponent(tempButton);
	SetCancelButton(tempButton);
	SetOkayButton(tempButton);
	currentY+=20;
	scrollPanel->InnerSize = ui::Point(Size.X, currentY);
}

void OptionsView::UpdateAmbientAirTempPreview(float airTemp, bool isValid)
{
	if (isValid)
	{
		int HeatToColour(float temp);
		int c = HeatToColour(airTemp);
		ambientAirTempPreview->Appearance.BackgroundInactive = ui::Colour(PIXR(c), PIXG(c), PIXB(c));
		ambientAirTempPreview->SetText("");
	}
	else
	{
		ambientAirTempPreview->Appearance.BackgroundInactive = ui::Colour(0, 0, 0);
		ambientAirTempPreview->SetText("?");
	}
	ambientAirTempPreview->Appearance.BackgroundHover = ambientAirTempPreview->Appearance.BackgroundInactive;
}

void OptionsView::UpdateAirTemp(String temp, bool isDefocus)
{
	// Parse air temp and determine validity
	float airTemp;
	bool isValid;
	try
	{
		void ParseFloatProperty(String value, float &out);
		ParseFloatProperty(temp, airTemp);
		isValid = true;
	}
	catch (const std::exception &ex)
	{
		isValid = false;
	}

	// While defocusing, correct out of range temperatures and empty textboxes
	if (isDefocus)
	{
		if (temp.empty())
		{
			isValid = true;
			airTemp = float(R_TEMP) + 273.15f;
		}
		else if (!isValid)
			return;
		else if (airTemp < MIN_TEMP)
			airTemp = MIN_TEMP;
		else if (airTemp > MAX_TEMP)
			airTemp = MAX_TEMP;
		else
			return;

		// Update textbox with the new value
		StringBuilder sb;
		sb << Format::Precision(2) << airTemp;
		ambientAirTemp->SetText(sb.Build());
	}
	// Out of range temperatures are invalid, preview should go away
	else if (airTemp < MIN_TEMP || airTemp > MAX_TEMP)
		isValid = false;

	// If valid, set temp
	if (isValid)
		c->SetAmbientAirTemperature(airTemp);

	UpdateAmbientAirTempPreview(airTemp, isValid);
}

void OptionsView::NotifySettingsChanged(OptionsModel * sender)
{
	heatSimulation->SetChecked(sender->GetHeatSimulation());
	ambientHeatSimulation->SetChecked(sender->GetAmbientHeatSimulation());
	newtonianGravity->SetChecked(sender->GetNewtonianGravity());
	waterEqualisation->SetChecked(sender->GetWaterEqualisation());
	airMode->SetOption(sender->GetAirMode());
	// Initialize air temp and preview only when the options menu is opened, and not when user is actively editing the textbox
	if (!initializedAirTempPreview)
	{
		initializedAirTempPreview = true;
		float airTemp = sender->GetAmbientAirTemperature();
		UpdateAmbientAirTempPreview(airTemp, true);
		StringBuilder sb;
		sb << Format::Precision(2) << airTemp;
		ambientAirTemp->SetText(sb.Build());
	}
	gravityMode->SetOption(sender->GetGravityMode());
	decoSpace->SetOption(sender->GetDecoSpace());
	edgeMode->SetOption(sender->GetEdgeMode());
	scale->SetOption(sender->GetScale());
	resizable->SetChecked(sender->GetResizable());
	fullscreen->SetChecked(sender->GetFullscreen());
	altFullscreen->SetChecked(sender->GetAltFullscreen());
	forceIntegerScaling->SetChecked(sender->GetForceIntegerScaling());
	fastquit->SetChecked(sender->GetFastQuit());
	showAvatars->SetChecked(sender->GetShowAvatars());
	mouseClickRequired->SetChecked(sender->GetMouseClickRequired());
	includePressure->SetChecked(sender->GetIncludePressure());
	perfectCirclePressure->SetChecked(sender->GetPerfectCircle());
	momentumScroll->SetChecked(sender->GetMomentumScroll());
}

void OptionsView::AttachController(OptionsController * c_)
{
	c = c_;
}

void OptionsView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

void OptionsView::OnTryExit(ExitMethod method)
{
	c->Exit();
}


OptionsView::~OptionsView() {
}
