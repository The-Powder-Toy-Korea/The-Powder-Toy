#include "RenderView.h"
#include "simulation/ElementGraphics.h"
#include "simulation/SimulationData.h"
#include "simulation/Simulation.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VideoBuffer.h"
#include "RenderController.h"
#include "RenderModel.h"
#include "gui/interface/Checkbox.h"
#include "gui/interface/Button.h"
#include "gui/game/GameController.h"
#include "gui/game/GameView.h"

class ModeCheckbox : public ui::Checkbox
{
public:
	using ui::Checkbox::Checkbox;
	uint32_t mode;
};

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, WINDOWH)),
	ren(nullptr),
	toolTip(""),
	isToolTipFadingIn(false)
{
	auto addPresetButton = [this](int index, Icon icon, ui::Point offset, String tooltip) {
		auto *presetButton = new ui::Button(ui::Point(XRES, YRES) + offset, ui::Point(30, 13), "", tooltip);
		presetButton->SetIcon(icon);
		presetButton->SetActionCallback({ [this, index] { c->LoadRenderPreset(index); } });
		AddComponent(presetButton);
	};
	addPresetButton( 1, IconVelocity  , ui::Point( -37,  6), "사전 설정된 기류 디스플레이 모드");
	addPresetButton( 2, IconPressure  , ui::Point( -37, 24), "사전 설정된 압력 디스플레이 모드");
	addPresetButton( 3, IconPersistant, ui::Point( -76,  6), "사전 설정된 잔상 디스플레이 모드");
	addPresetButton( 4, IconFire      , ui::Point( -76, 24), "사전 설정된 불꽃 디스플레이 모드");
	addPresetButton( 5, IconBlob      , ui::Point(-115,  6), "사전 설정된 얼룩 디스플레이 모드");
	addPresetButton( 6, IconHeat      , ui::Point(-115, 24), "사전 설정된 열 디스플레이 모드");
	addPresetButton( 7, IconBlur      , ui::Point(-154,  6), "사전 설정된 화려한 디스플레이 모드");
	addPresetButton( 8, IconBasic     , ui::Point(-154, 24), "사전 설정된 기본 디스플레이 모드");
	addPresetButton( 9, IconGradient  , ui::Point(-193,  6), "사전 설정된 열 명암 디스플레이 모드");
	addPresetButton( 0, IconAltAir    , ui::Point(-193, 24), "사전 설정된 대체 기류 디스플레이 모드");
	addPresetButton(10, IconLife      , ui::Point(-232,  6), "사전 설정된 수명 명암 디스플레이 모드");

	auto addRenderModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *renderModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		renderModes.push_back(renderModeCheckbox);
		renderModeCheckbox->mode = mode;
		renderModeCheckbox->SetIcon(icon);
		renderModeCheckbox->SetActionCallback({ [this] {
			auto renderMode = CalculateRenderMode();
			c->SetRenderMode(renderMode);
		} });
		AddComponent(renderModeCheckbox);
	};
	addRenderModeCheckbox(RENDER_EFFE, IconEffect, ui::Point( 1,  4), "일부 요소에 특별한 화염 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_FIRE, IconFire  , ui::Point( 1, 22), "기체 요소에 불 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_GLOW, IconGlow  , ui::Point(33,  4), "일부 요소에 발광 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_BLUR, IconBlur  , ui::Point(33, 22), "액체 요소에 흐리기 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_BLOB, IconBlob  , ui::Point(65,  4), "모든 것을 얼룩처럼 표시합니다.");
	addRenderModeCheckbox(RENDER_BASC, IconBasic , ui::Point(65, 22), "기본 렌더링이며, 이 렌더링이 없으면 대부분의 사물이 보이지 않습니다.");
	addRenderModeCheckbox(RENDER_SPRK, IconEffect, ui::Point(97,  4), "전류에 발광 효과를 표시합니다.");

	auto addDisplayModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *displayModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		displayModes.push_back(displayModeCheckbox);
		displayModeCheckbox->mode = mode;
		displayModeCheckbox->SetIcon(icon);
		displayModeCheckbox->SetActionCallback({ [this, displayModeCheckbox] {
			auto displayMode = c->GetDisplayMode();
			// Air display modes are mutually exclusive
			if (displayModeCheckbox->mode & DISPLAY_AIR)
			{
				displayMode &= ~DISPLAY_AIR;
			}
			if (displayModeCheckbox->GetChecked())
			{
				displayMode |= displayModeCheckbox->mode;
			}
			else
			{
				displayMode &= ~displayModeCheckbox->mode;
			}
			c->SetDisplayMode(displayMode);
		} });
		AddComponent(displayModeCheckbox);
	};
	line1 = 130;
	addDisplayModeCheckbox(DISPLAY_AIRC, IconAltAir    , ui::Point(135,  4), "압력을 적청으로, 기류를 하얗게 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_AIRP, IconPressure  , ui::Point(135, 22), "압력을 표시합니다. 음압은 빨간색으로, 양압은 파란색으로 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_AIRV, IconVelocity  , ui::Point(167,  4), "기류와 양압을 표시합니다. 상하 방향은 파란색, 좌우 방향은 빨간색, 양압은 초록색으로 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_AIRH, IconHeat      , ui::Point(167, 22), "공기의 온도를 열 디스플레이와 같이 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_AIRW, IconVort      , ui::Point(199,  4), "와도를 표시합니다. 시계 방향은 빨간색, 시계 반대 방향은 파란색으로 표시합니다.");
	line2 = 232;
	addDisplayModeCheckbox(DISPLAY_WARP, IconWarp      , ui::Point(237, 22), "중력 렌즈 렌더링이며, 뉴턴 중력의 빛 왜곡 효과를 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_EFFE, IconEffect    , ui::Point(237,  4), "고체 이동 활성화, 스틱맨 총, Premium\xE2\x84\xA2 그래픽(패러디)");
	addDisplayModeCheckbox(DISPLAY_PERS, IconPersistant, ui::Point(269,  4), "입자 경로가 화면에 잠시 동안 지속됩니다.");
	line3 = 302;

	auto addColourModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *colourModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		colourModes.push_back(colourModeCheckbox);
		colourModeCheckbox->mode = mode;
		colourModeCheckbox->SetIcon(icon);
		colourModeCheckbox->SetActionCallback({ [this, colourModeCheckbox] {
			auto colorMode = c->GetColorMode();
			// exception: looks like an independent set of settings but behaves more like an index
			if (colourModeCheckbox->GetChecked())
			{
				colorMode = colourModeCheckbox->mode;
			}
			else
			{
				colorMode = 0;
			}
			c->SetColorMode(colorMode);
		} });
		AddComponent(colourModeCheckbox);
	};
	addColourModeCheckbox(COLOUR_HEAT, IconHeat    , ui::Point(307,  4), "요소의 온도를 표시하며, 짙은 남색이 가장 차가움을, 진분홍색이 가장 뜨거움을 나타냅니다.");
	addColourModeCheckbox(COLOUR_LIFE, IconLife    , ui::Point(307, 22), "요소의 수명값을 명암으로 나타냅니다.");
	addColourModeCheckbox(COLOUR_GRAD, IconGradient, ui::Point(339, 22), "요소의 색상을 약간 변경하여 열 확산을 나타냅니다.");
	addColourModeCheckbox(COLOUR_BASC, IconBasic   , ui::Point(339,  4), "다른 모든 설정과 도색을 무시하며, 특수 효과를 전혀 사용하지 않습니다.");
	line4 = 372;
}

uint32_t RenderView::CalculateRenderMode()
{
	uint32_t renderMode = 0;
	for (auto &checkbox : renderModes)
	{
		if (checkbox->GetChecked())
			renderMode |= checkbox->mode;
	}

	return renderMode;
}

void RenderView::OnMouseDown(int x, int y, unsigned button)
{
	if(x > XRES || y < YRES)
		c->Exit();
}

void RenderView::OnTryExit(ExitMethod method)
{
	c->Exit();
}

void RenderView::NotifyRendererChanged(RenderModel * sender)
{
	ren = sender->GetRenderer();
	rendererSettings = sender->GetRendererSettings();
}

void RenderView::NotifySimulationChanged(RenderModel * sender)
{
	sim = sender->GetSimulation();
}

void RenderView::NotifyRenderChanged(RenderModel * sender)
{
	for (size_t i = 0; i < renderModes.size(); i++)
	{
		//Compares bitmasks at the moment, this means that "Point" is always on when other options that depend on it are, this might confuse some users, TODO: get the full list and compare that?
		auto renderMode = renderModes[i]->mode;
		renderModes[i]->SetChecked(renderMode == (sender->GetRenderMode() & renderMode));
	}
}

void RenderView::NotifyDisplayChanged(RenderModel * sender)
{
	for (size_t i = 0; i < displayModes.size(); i++)
	{
		auto displayMode = displayModes[i]->mode;
		displayModes[i]->SetChecked(displayMode == (sender->GetDisplayMode() & displayMode));
	}
}

void RenderView::NotifyColourChanged(RenderModel * sender)
{
	for (size_t i = 0; i < colourModes.size(); i++)
	{
		auto colorMode = colourModes[i]->mode;
		colourModes[i]->SetChecked(colorMode == sender->GetColorMode());
	}
}

void RenderView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(WINDOW.OriginRect(), 0x000000_rgb);
	auto *view = GameController::Ref().GetView();
	view->PauseRendererThread();
	ren->ApplySettings(*rendererSettings);
	view->RenderSimulation(*sim, true);
	view->AfterSimDraw(*sim);
	for (auto y = 0; y < YRES; ++y)
	{
		auto &video = ren->GetVideo();
		std::copy_n(video.data() + video.Size().X * y, video.Size().X, g->Data() + g->Size().X * y);
	}
	g->DrawLine({ 0, YRES }, { XRES-1, YRES }, 0xC8C8C8_rgb);
	g->DrawLine({ line1, YRES }, { line1, WINDOWH }, 0xC8C8C8_rgb);
	g->DrawLine({ line2, YRES }, { line2, WINDOWH }, 0xC8C8C8_rgb);
	g->DrawLine({ line3, YRES }, { line3, WINDOWH }, 0xC8C8C8_rgb);
	g->DrawLine({ line4, YRES }, { line4, WINDOWH }, 0xC8C8C8_rgb);
	g->DrawLine({ XRES, 0 }, { XRES, WINDOWH }, 0xFFFFFF_rgb);
	if(toolTipPresence && toolTip.length())
	{
		g->BlendText({ 6, Size.Y-MENUSIZE-12 }, toolTip, 0xFFFFFF_rgb .WithAlpha(toolTipPresence>51?255:toolTipPresence*5));
	}
}

void RenderView::OnTick()
{
	if (isToolTipFadingIn)
	{
		isToolTipFadingIn = false;
		toolTipPresence.SetTarget(120);
	}
	else
	{
		toolTipPresence.SetTarget(0);
	}
}

void RenderView::OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (repeat)
		return;
	if (shift && key == '1')
		c->LoadRenderPreset(10);
	else if(key >= '0' && key <= '9')
	{
		c->LoadRenderPreset(key-'0');
	}
}

void RenderView::ToolTip(ui::Point senderPosition, String toolTip)
{
	this->toolTip = toolTip;
	this->isToolTipFadingIn = true;
}

RenderView::~RenderView() {
}
