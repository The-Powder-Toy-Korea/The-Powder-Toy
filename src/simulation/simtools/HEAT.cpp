#include "simulation/ToolCommon.h"

static int perform(SimTool *tool, Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_HEAT()
{
	Identifier = "DEFAULT_TOOL_HEAT";
	Name = "HEAT";
	Colour = 0xFFDD00_rgb;
	Description = "가열 도구: 요소를 가열합니다.";
	Perform = &perform;
}

static int perform(SimTool *tool, Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	if(!cpart)
		return 0;
	if (cpart->type == PT_PUMP || cpart->type == PT_GPMP)
		cpart->temp += strength*.1f;
	else
		cpart->temp += strength*2.0f;

	if (cpart->temp > MAX_TEMP)
		cpart->temp = MAX_TEMP;
	else if (cpart->temp < 0)
		cpart->temp = 0;
	return 1;
}
