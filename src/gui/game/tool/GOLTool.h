#pragma once
#include "Tool.h"

class GameModel;

class GOLTool: public Tool
{
	GameModel &gameModel;
public:
	GOLTool(GameModel &gameModel):
		Tool(0, "CUST", "새 사용자 지정 생명 게임을 추가합니다. [Ctrl] + [Shift]를 누른 상태로 우클릭하여 제거합니다.",
			0xFEA900_rgb, "DEFAULT_UI_ADDLIFE", NULL
		),
		gameModel(gameModel)
	{
		MenuSection = SC_LIFE;
	}

	void OpenWindow(Simulation *sim, int toolSelection, int rule = 0, RGB<uint8_t> colour1 = 0x000000_rgb, RGB<uint8_t> colour2 = 0x000000_rgb);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override { };
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override { };
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { };
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { };

	void Select(int toolSelection) final override;
};
