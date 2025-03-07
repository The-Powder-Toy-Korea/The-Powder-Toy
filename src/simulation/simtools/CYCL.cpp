#include "simulation/ToolCommon.h"
#include "simulation/Air.h"

#include <cmath>

static int perform(SimTool *tool, Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength);

void SimTool::Tool_CYCL()
{
	Identifier = "DEFAULT_TOOL_CYCL";
	Name = "CYCL";
	Colour = 0x132f5b_rgb;
	Description = "사이클론: 공기를 섞습니다.";
	Perform = &perform;
}

static int perform(SimTool *tool, Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	/*
		Air velocity calculation.
		(x, y) -- turn 90 deg -> (-y, x)
	*/
	// only trigger once per cell (less laggy)
	if ((x%CELL) == 0 && (y%CELL) == 0)
	{
		if(brushX == x && brushY == y)
			return 1;

		float *vx = &sim->vx[y / CELL][x / CELL];
		float *vy = &sim->vy[y / CELL][x / CELL];

		auto dvx = float(brushX - x);
		auto dvy = float(brushY - y);
		float invsqr = 1/sqrtf(dvx*dvx + dvy*dvy);

		*vx -= (strength / 16) * (-dvy)*invsqr;
		*vy -= (strength / 16) * dvx*invsqr;

		// Clamp velocities
		if (*vx > MAX_PRESSURE)
			*vx = MAX_PRESSURE;
		else if (*vx < MIN_PRESSURE)
			*vx = MIN_PRESSURE;
		if (*vy > MAX_PRESSURE)
			*vy = MAX_PRESSURE;
		else if (*vy < MIN_PRESSURE)
			*vy = MIN_PRESSURE;

	}

	return 1;
}
