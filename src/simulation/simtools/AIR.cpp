#include "simulation/ToolCommon.h"
#include "simulation/Air.h"

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_AIR()
{
	Identifier = "DEFAULT_TOOL_AIR";
	Name = "AIR";
	Colour = 0xFFFFFF_rgb;
	Description = "공기: 압력을 생성합니다.";
	Perform = &perform;
}

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	sim->air->pv[y/CELL][x/CELL] += strength*0.05f;

	if (sim->air->pv[y/CELL][x/CELL] > MAX_PRESSURE)
		sim->air->pv[y/CELL][x/CELL] = MAX_PRESSURE;
	else if (sim->air->pv[y/CELL][x/CELL] < MIN_PRESSURE)
		sim->air->pv[y/CELL][x/CELL] = MIN_PRESSURE;
	return 1;
}
