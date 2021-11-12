#include "RenderView.h"

#include "simulation/ElementGraphics.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "RenderController.h"
#include "RenderModel.h"

#include "gui/interface/Checkbox.h"
#include "gui/interface/Button.h"

class ModeCheckbox : public ui::Checkbox
{
public:
	using ui::Checkbox::Checkbox;
	unsigned int mode;
};

RenderView::RenderView():
	ui::Window(ui::Point(0, 0), ui::Point(XRES, WINDOWH)),
	ren(NULL),
	toolTip(""),
	toolTipPresence(0),
	isToolTipFadingIn(false)
{
	auto addPresetButton = [this](int index, Icon icon, ui::Point offset, String tooltip) {
		auto *presetButton = new ui::Button(ui::Point(XRES, YRES) + offset, ui::Point(30, 13), "", tooltip);
		presetButton->SetIcon(icon);
		presetButton->SetActionCallback({ [this, index] { c->LoadRenderPreset(index); } });
		AddComponent(presetButton);
	};
	addPresetButton( 1, IconVelocity  , ui::Point( -37,  6), "사전 설정된 속도 디스플레이 모드");
	addPresetButton( 2, IconPressure  , ui::Point( -37, 24), "사전 설정된 압력 디스플레이 모드");
	addPresetButton( 3, IconPersistant, ui::Point( -76,  6), "사전 설정된 꼬리 디스플레이 모드");
	addPresetButton( 4, IconFire      , ui::Point( -76, 24), "사전 설정된 불 디스플레이 모드");
	addPresetButton( 5, IconBlob      , ui::Point(-115,  6), "사전 설정된 블롭 디스플레이 모드");
	addPresetButton( 6, IconHeat      , ui::Point(-115, 24), "사전 설정된 열 디스플레이 모드");
	addPresetButton( 7, IconBlur      , ui::Point(-154,  6), "사전 설정된 화려한 디스플레이 모드");
	addPresetButton( 8, IconBasic     , ui::Point(-154, 24), "사전 설정된 일반 디스플레이 모드");
	addPresetButton( 9, IconGradient  , ui::Point(-193,  6), "사전 설정된 열 그라데이션 디스플레이 모드");
	addPresetButton( 0, IconAltAir    , ui::Point(-193, 24), "사전 설정된 대체 속도 디스플레이 모드");
	addPresetButton(10, IconLife      , ui::Point(-232,  6), "사전 설정된 Life 디스플레이 모드");

	auto addRenderModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *renderModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		renderModes.push_back(renderModeCheckbox);
		renderModeCheckbox->mode = mode;
		renderModeCheckbox->SetIcon(icon);
		renderModeCheckbox->SetActionCallback({ [this, renderModeCheckbox] {
			if (renderModeCheckbox->GetChecked())
				c->SetRenderMode(renderModeCheckbox->mode);
			else
				c->UnsetRenderMode(renderModeCheckbox->mode);
		} });
		AddComponent(renderModeCheckbox);
	};
	addRenderModeCheckbox(RENDER_EFFE, IconEffect, ui::Point( 1,  4), "일부 물질에 특별한 화염 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_FIRE, IconFire  , ui::Point( 1, 22), "기체 물질에 불 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_GLOW, IconGlow  , ui::Point(33,  4), "일부 물질에 조명 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_BLUR, IconBlur  , ui::Point(33, 22), "액체 물질에 블러 효과를 표시합니다.");
	addRenderModeCheckbox(RENDER_BLOB, IconBlob  , ui::Point(65,  4), "모든 것의 윤곽선을 흐리게 표시합니다.");
	addRenderModeCheckbox(RENDER_BASC, IconBasic , ui::Point(65, 22), "기본 렌더링, 이 렌더링이 있어야 대부분의 것들이 표시됩니다.");
	addRenderModeCheckbox(RENDER_SPRK, IconEffect, ui::Point(97,  4), "전류에 조명 효과를 표시합니다.");

	auto addDisplayModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *displayModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		displayModes.push_back(displayModeCheckbox);
		displayModeCheckbox->mode = mode;
		displayModeCheckbox->SetIcon(icon);
		displayModeCheckbox->SetActionCallback({ [this, displayModeCheckbox] {
			if (displayModeCheckbox->GetChecked())
				c->SetDisplayMode(displayModeCheckbox->mode);
			else
				c->UnsetDisplayMode(displayModeCheckbox->mode);
		} });
		AddComponent(displayModeCheckbox);
	};
	line1 = 130;
	addDisplayModeCheckbox(DISPLAY_AIRC, IconAltAir  , ui::Point(135,  4), "압력을 적청으로, 속도를 하얗게 표시합니다.");
	addDisplayModeCheckbox(DISPLAY_AIRP, IconPressure, ui::Point(135, 22), "압력을 표시합니다. 음압은 빨간색으로, 양압은 파란색으로 표시됩니다.");
	addDisplayModeCheckbox(DISPLAY_AIRV, IconVelocity, ui::Point(167,  4), "속도와 양압을 표시합니다. 위/아래는 파란색, 왼쪽/오른쪽은 빨간색, 양압은 초록색으로 표시됩니다.");
	addDisplayModeCheckbox(DISPLAY_AIRH, IconHeat    , ui::Point(167, 22), "공기의 온도를 열 디스플레이와 같이 표시합니다.");
	line2 = 200;
	addDisplayModeCheckbox(DISPLAY_WARP, IconWarp    , ui::Point(205, 22), "중력 렌즈 디스플레이: 뉴턴 중력이 발생하는 곳에서 빛을 왜곡하는 효과를 표시합니다.");
#ifdef OGLR
# define TOOLTIP "OpenGL 효과 중 하나로 추정됨\xe2\x80\xa6"
#else
# define TOOLTIP "고체 이동 활성화, 스틱맨 총, 프리미엄\xE2\x84\xA2 그래픽 (패러디)"
#endif
	addDisplayModeCheckbox(DISPLAY_EFFE, IconEffect    , ui::Point(205,  4), TOOLTIP);
#undef TOOLTIP
	addDisplayModeCheckbox(DISPLAY_PERS, IconPersistant, ui::Point(237,  4), "물질이 지나간 곳에 잠시동안 경로를 표시합니다.");
	line3 = 270;

	auto addColourModeCheckbox = [this](unsigned int mode, Icon icon, ui::Point offset, String tooltip) {
		auto *colourModeCheckbox = new ModeCheckbox(ui::Point(0, YRES) + offset, ui::Point(30, 16), "", tooltip);
		colourModes.push_back(colourModeCheckbox);
		colourModeCheckbox->mode = mode;
		colourModeCheckbox->SetIcon(icon);
		colourModeCheckbox->SetActionCallback({ [this, colourModeCheckbox] {
			if(colourModeCheckbox->GetChecked())
				c->SetColourMode(colourModeCheckbox->mode);
			else
				c->SetColourMode(0);
		} });
		AddComponent(colourModeCheckbox);
	};
	addColourModeCheckbox(COLOUR_HEAT, IconHeat    , ui::Point(275,  4), "물질의 온도를 표시합니다. 짙은 남색이 가장 차갑고, 진분홍색이 가장 뜨거움을 나타냅니다.");
	addColourModeCheckbox(COLOUR_LIFE, IconLife    , ui::Point(275, 22), "물질의 Life 값을 회색조로 표시합니다.");
	addColourModeCheckbox(COLOUR_GRAD, IconGradient, ui::Point(307, 22), "열 확산을 자세히 표시하기 위해 물질의 색상을 약간 변경합니다.");
	addColourModeCheckbox(COLOUR_BASC, IconBasic   , ui::Point(307,  4), "모든 물질에 대한 특수 효과나 도색 등을 모두 무효화합니다.");
	line4 = 340;
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
		auto colourMode = colourModes[i]->mode;
		colourModes[i]->SetChecked(colourMode == sender->GetColourMode());
	}
}

void RenderView::OnDraw()
{
	Graphics * g = GetGraphics();
	g->clearrect(-1, -1, WINDOWW+1, WINDOWH+1);
	if(ren)
	{
		ren->clearScreen(1.0f);
		ren->RenderBegin();
		ren->RenderEnd();
	}
	g->draw_line(0, YRES, XRES-1, YRES, 200, 200, 200, 255);
	g->draw_line(line1, YRES, line1, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line2, YRES, line2, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line3, YRES, line3, WINDOWH, 200, 200, 200, 255);
	g->draw_line(line4, YRES, line4, WINDOWH, 200, 200, 200, 255);
	g->draw_line(XRES, 0, XRES, WINDOWH, 255, 255, 255, 255);
	if(toolTipPresence && toolTip.length())
	{
		g->drawtext(6, Size.Y-MENUSIZE-12, toolTip, 255, 255, 255, toolTipPresence>51?255:toolTipPresence*5);
	}
}

void RenderView::OnTick(float dt)
{
	if (isToolTipFadingIn)
	{
		isToolTipFadingIn = false;
		if(toolTipPresence < 120)
		{
			toolTipPresence += int(dt*2)>1?int(dt*2):2;
			if(toolTipPresence > 120)
				toolTipPresence = 120;
		}
	}
	if(toolTipPresence>0)
	{
		toolTipPresence -= int(dt)>0?int(dt):1;
		if(toolTipPresence<0)
			toolTipPresence = 0;
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
