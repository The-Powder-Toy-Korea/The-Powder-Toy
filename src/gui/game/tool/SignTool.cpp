#include "SignTool.h"
#include "simulation/Simulation.h"
#include "gui/Style.h"
#include "gui/interface/Window.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/DropDown.h"
#include "gui/game/GameModel.h"
#include "graphics/Graphics.h"

class SignWindow: public ui::Window
{
public:
	ui::DropDown * justification;
	ui::Textbox * textField;
	SignTool * tool;
	sign * movingSign;
	bool signMoving;
	Simulation * sim;
	int signID;
	ui::Point signPosition;
	SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_);
	void OnDraw() override;
	void DoDraw() override;
	void DoMouseMove(int x, int y, int dx, int dy) override;
	void DoMouseDown(int x, int y, unsigned button) override;
	void DoMouseUp(int x, int y, unsigned button) override
	{
		if(!signMoving)
			ui::Window::DoMouseUp(x, y, button);
	}
	void DoMouseWheel(int x, int y, int d) override
	{
		if(!signMoving)
			ui::Window::DoMouseWheel(x, y, d);
	}
	void DoKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override
	{
		if(!signMoving)
			ui::Window::DoKeyPress(key, scan, repeat, shift, ctrl, alt);
	}
	void DoKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override
	{
		if(!signMoving)
			ui::Window::DoKeyRelease(key, scan, repeat, shift, ctrl, alt);
	}
	virtual ~SignWindow() {}
	void OnTryExit(ui::Window::ExitMethod method) override;
};

SignWindow::SignWindow(SignTool * tool_, Simulation * sim_, int signID_, ui::Point position_):
	ui::Window(ui::Point(-1, -1), ui::Point(250, 87)),
	tool(tool_),
	movingSign(nullptr),
	signMoving(false),
	sim(sim_),
	signID(signID_),
	signPosition(position_)
{
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), "새 표지판");
	messageLabel->SetTextColour(style::Colour::InformationTitle);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(messageLabel);

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "확인");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = (ui::Colour(200, 200, 200));
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		if(signID==-1 && textField->GetText().length())
		{
			sim->signs.push_back(sign(textField->GetText(), signPosition.X, signPosition.Y, (sign::Justification)justification->GetOption().second));
		}
		else if(signID!=-1 && textField->GetText().length())
		{
			sim->signs[signID] = sign(sign(textField->GetText(), signPosition.X, signPosition.Y, (sign::Justification)justification->GetOption().second));
		}
		SelfDestruct();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	ui::Label * tempLabel = new ui::Label(ui::Point(8, 48), ui::Point(40, 15), "꼬리:");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(tempLabel);

	justification = new ui::DropDown(ui::Point(52, 48), ui::Point(55, 16));
	AddComponent(justification);
	justification->AddOption(std::pair<String, int>(0xE020 + String(" 좌측"), (int)sign::Left));
	justification->AddOption(std::pair<String, int>(0xE01E + String(" 중앙"), (int)sign::Middle));
	justification->AddOption(std::pair<String, int>(0xE01F + String(" 우측"), (int)sign::Right));
	justification->AddOption(std::pair<String, int>(0xE01D + String(" 없음"), (int)sign::None));
	justification->SetOption(1);
	justification->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;

	textField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 17), "", "내용");
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->SetLimit(45);
	textField->SetActionCallback({ [this] {
		if (signID!=-1)
		{
			sim->signs[signID].text = textField->GetText();
			sim->signs[signID].ju = (sign::Justification)justification->GetOption().second;
		}
	} });
	AddComponent(textField);
	FocusComponent(textField);

	if(signID!=-1)
	{
		messageLabel->SetText("Edit sign");

		textField->SetText(sim->signs[signID].text);
		justification->SetOption(sim->signs[signID].ju);

		ui::Point position = ui::Point(justification->Position.X+justification->Size.X+3, 48);
		ui::Button * moveButton = new ui::Button(position, ui::Point(((Size.X-position.X-8)/2)-2, 16), "이동");
		moveButton->SetActionCallback({ [this] {
			if (signID!=-1)
			{
				movingSign = &sim->signs[signID];
				sim->signs[signID].ju = (sign::Justification)justification->GetOption().second;
				signMoving = true;
			}
		} });
		AddComponent(moveButton);

		position = ui::Point(justification->Position.X+justification->Size.X+3, 48)+ui::Point(moveButton->Size.X+3, 0);
		ui::Button * deleteButton = new ui::Button(position, ui::Point((Size.X-position.X-8)-1, 16), "제거");
		//deleteButton->SetIcon(IconDelete);
		deleteButton->SetActionCallback({ [this] {
			CloseActiveWindow();
			if (signID!=-1)
			{
				sim->signs.erase(sim->signs.begin() + signID);
			}
			SelfDestruct();
		} });

		signPosition.X = sim->signs[signID].x;
		signPosition.Y = sim->signs[signID].y;

		AddComponent(deleteButton);
	}

	MakeActiveWindow();
}

void SignWindow::OnTryExit(ui::Window::ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

void SignWindow::DoDraw()
{
	for (auto &currentSign : sim->signs)
	{
		int x, y, w, h, dx, dy;
		Graphics * g = GetGraphics();

		String text = currentSign.getDisplayText(sim, x, y, w, h);
		g->DrawFilledRect(RectSized(Vec2{ x + 1, y + 1 }, Vec2{ w, h - 1 }), 0x000000_rgb);
		g->DrawRect(RectSized(Vec2{ x, y }, Vec2{ w+1, h }), 0xC0C0C0_rgb);
		g->BlendText({ x+3, y+4 }, text, 0xFFFFFF_rgb .WithAlpha(255));

		if (currentSign.ju != sign::None)
		{
			x = currentSign.x;
			y = currentSign.y;
			dx = 1 - currentSign.ju;
			dy = (currentSign.y > 18) ? -1 : 1;
			for (int j=0; j<4; j++)
			{
				g->DrawPixel({ x, y }, 0xC0C0C0_rgb);
				x+=dx;
				y+=dy;
			}
		}
	}
	if(!signMoving)
	{
		ui::Window::DoDraw();
	}
}

void SignWindow::DoMouseMove(int x, int y, int dx, int dy) {
	if(!signMoving)
		ui::Window::DoMouseMove(x, y, dx, dy);
	else
	{
		ui::Point pos = tool->gameModel.AdjustZoomCoords(ui::Point(x, y));
		if(pos.X < XRES && pos.Y < YRES)
		{
			movingSign->x = pos.X;
			movingSign->y = pos.Y;
			signPosition.X = pos.X;
			signPosition.Y = pos.Y;
		}
	}
}

void SignWindow::DoMouseDown(int x, int y, unsigned button)
{
	if(!signMoving)
		ui::Window::DoMouseDown(x, y, button);
	else
	{
		signMoving = false;
	}
}

void SignWindow::OnDraw()
{
	Graphics * g = GetGraphics();

	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xC8C8C8_rgb);
}

std::unique_ptr<VideoBuffer> SignTool::GetIcon(int toolID, Vec2<int> size)
{
	auto texture = std::make_unique<VideoBuffer>(size);
	texture->DrawRect(size.OriginRect(), 0xA0A0A0_rgb);
	texture->BlendChar((size / 2) - Vec2(5, 5), 0xE021, 0x204080_rgb .WithAlpha(0xFF));
	texture->BlendChar((size / 2) - Vec2(5, 5), 0xE020, 0xFFFFFF_rgb .WithAlpha(0xFF));
	return texture;
}

void SignTool::Click(Simulation * sim, Brush const &brush, ui::Point position)
{
	int signX, signY, signW, signH, signIndex = -1;
	for (size_t i = 0; i < sim->signs.size(); i++)
	{
		sim->signs[i].getDisplayText(sim, signX, signY, signW, signH);
		if (position.X > signX && position.X < signX+signW && position.Y > signY && position.Y < signY+signH)
		{
			signIndex = i;
			break;
		}
	}
	if (signIndex != -1 || sim->signs.size() < MAXSIGNS)
		new SignWindow(this, sim, signIndex, position);
}
