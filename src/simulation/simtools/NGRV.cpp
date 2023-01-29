#include "simulation/ToolCommon.h"

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushYy, float strength);

void SimTool::Tool_NGRV()
{
	Identifier = "DEFAULT_TOOL_NGRV";
	Name = "NGRV";
	Colour = PIXPACK(0xAACCFF);
	Description = "순간적인 방출 중력을 생성합니다.";
	Perform = &perform;
}

static int perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushYy, float strength)
{
	sim->gravmap[((y/CELL)*XCELLS)+(x/CELL)] = strength*-5.0f;
	return 1;
}
