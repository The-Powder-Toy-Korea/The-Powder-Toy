#pragma once
#include "simulation/AccessProperty.h"
#include "Tool.h"
#include <optional>

class GameModel;

class PropertyTool: public Tool
{
public:
	struct Configuration
	{
		AccessProperty changeProperty;
		String propertyValueStr;
	};

private:
	void SetProperty(Simulation *sim, ui::Point position);
	void SetConfiguration(std::optional<Configuration> newConfiguration);

	GameModel &gameModel;
	std::optional<Configuration> configuration;

	friend class PropertyWindow;

public:
	PropertyTool(GameModel &newGameModel):
		Tool(0, "PROP", "속성 그리기 도구: 그리기 공간에 있는 요소의 속성을 바꾸는 데 사용합니다.",
			0xFEA900_rgb, "DEFAULT_UI_PROPERTY", NULL
		), gameModel(newGameModel)
	{}

	void OpenWindow(Simulation *sim, std::optional<int> takePropertyFrom);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;

	std::optional<Configuration> GetConfiguration() const
	{
		return configuration;
	}

	void Select(int toolSelection) final override;
};
