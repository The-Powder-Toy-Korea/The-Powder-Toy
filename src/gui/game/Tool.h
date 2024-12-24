#pragma once
#include "common/String.h"
#include "common/Vec2.h"
#include "graphics/Pixel.h"
#include "graphics/VideoBuffer.h"
#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"
#include <memory>
#include <optional>

class Simulation;
class Brush;
struct Particle;

class Tool
{
private:
	std::unique_ptr<VideoBuffer> (*const textureGen)(int, Vec2<int>);

public:
	int const ToolID;
	String const Name;
	String const Description;
	ByteString const Identifier;
	RGB<uint8_t> const Colour;
	bool const Blocky;
	float Strength = 1.0f;
	bool shiftBehaviour = false;
	bool ctrlBehaviour = false;
	bool altBehaviour = false;

	Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL, bool blocky = false
	):
		textureGen(textureGen),
		ToolID(id),
		Name(name),
		Description(description),
		Identifier(identifier),
		Colour(colour),
		Blocky(blocky)
	{}

	virtual ~Tool()
	{}

	std::unique_ptr<VideoBuffer> GetTexture(Vec2<int>);
	virtual void Click(Simulation * sim, Brush const &brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush const &brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush const &brush, ui::Point position);
};

class GameModel;

class SignTool: public Tool
{
	GameModel &gameModel;

	friend class SignWindow;

public:
	SignTool(GameModel &model):
		Tool(0, "SIGN", "표지판: 텍스트를 표시합니다. 표지판을 클릭하여 편집하거나 다른 곳을 클릭하여 새 표지판을 생성합니다.",
			0x000000_rgb, "DEFAULT_UI_SIGN", SignTool::GetIcon
		),
		gameModel(model)
	{}

	virtual ~SignTool()
	{}

	static std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class SampleTool: public Tool
{
	GameModel &gameModel;

public:
	SampleTool(GameModel &model):
		Tool(0, "SMPL", "화면의 물질을 스포이트합니다.",
			0x000000_rgb, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon
		),
		gameModel(model)
	{}

	static std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);
	virtual ~SampleTool() {}
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class PropertyTool: public Tool
{
public:
	struct Configuration
	{
		StructProperty prop;
		PropertyValue propValue;
		bool changeType;
		int propertyIndex;
		String propertyValueStr;
	};

private:
	void SetProperty(Simulation *sim, ui::Point position);
	void SetConfiguration(std::optional<Configuration> newConfiguration);

	GameModel &gameModel;
	std::optional<Configuration> configuration;

	friend class PropertyWindow;

public:
	PropertyTool(GameModel &model):
		Tool(0, "PROP", "속성 그리기 도구: 물질의 속성을 바꾸는데 사용합니다.",
			0xFEA900_rgb, "DEFAULT_UI_PROPERTY", NULL
		),
		gameModel(model)
	{}

	virtual ~PropertyTool()
	{}

	void OpenWindow(Simulation *sim, const Particle *takePropertyFrom);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;

	std::optional<Configuration> GetConfiguration() const
	{
		return configuration;
	}
};

class GOLTool: public Tool
{
	GameModel &gameModel;
public:
	GOLTool(GameModel &gameModel):
		Tool(0, "CUST", "새 사용자 지정 생명 게임을 추가합니다(<Ctrl+Shift>를 누른 상태로 우클릭하여 제거합니다).",
			0xFEA900_rgb, "DEFAULT_UI_ADDLIFE", NULL
		),
		gameModel(gameModel)
	{}

	virtual ~GOLTool()
	{}

	void OpenWindow(Simulation *sim, int toolSelection, int rule = 0, RGB<uint8_t> colour1 = 0x000000_rgb, RGB<uint8_t> colour2 = 0x000000_rgb);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override { };
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { };
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { };
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { };
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		Tool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~ElementTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~Element_LIGH_Tool()
	{}

	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~Element_TESC_Tool()
	{}

	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~PlopTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		Tool(id, "", description, colour, identifier, textureGen, true)
	{
	}

	virtual ~WallTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class WindTool: public Tool
{
public:
	WindTool():
		Tool(0, "WIND", "공기 흐름을 생성합니다.",
			0x404040_rgb, "DEFAULT_UI_WIND")
	{}

	virtual ~WindTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};
