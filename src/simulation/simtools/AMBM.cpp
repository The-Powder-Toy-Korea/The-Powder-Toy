#include "simulation/ToolCommon.h"

static int perform(SimTool *tool, Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_AMBM()
{
	Identifier = "DEFAULT_TOOL_AMBM";
	Name = "AMBM";
	Colour = 0x00DDFF_rgb;
	Description = "복사열 감소 도구: 공기 온도를 낮춥니다.";
	Perform = &perform;
}

static int perform(SimTool *tool, Simulation *sim, Particle *cpart, int x, int y, int brushX, int brushY, float strength)
{
	if (!sim->aheat_enable)
	{
		return 0;
	}

	sim->hv[y / CELL][x / CELL] -= strength * 2.0f;
	if (sim->hv[y / CELL][x / CELL] > MAX_TEMP) sim->hv[y / CELL][x / CELL] = MAX_TEMP;
	if (sim->hv[y / CELL][x / CELL] < MIN_TEMP) sim->hv[y / CELL][x / CELL] = MIN_TEMP;
	return 1;
}
