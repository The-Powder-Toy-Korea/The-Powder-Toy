#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ISZS()
{
	Identifier = "DEFAULT_PT_ISZS";
	Name = "ISZS";
	Colour = 0x662089_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.0007f* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = 140.00f;
	HeatConduct = 251;
	Description = "고체 동위원소-Z: 고체 형태의 ISOZ로, 천천히 광자로 붕괴합니다.";

	Properties = TYPE_SOLID | PROP_PHOTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 300.0f;
	HighTemperatureTransition = PT_ISOZ;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	float rr, rrr;
	if (sim->rng.chance(1, 200) && sim->rng.chance(int(-4.0f * sim->pv[y/CELL][x/CELL]), 1000))
	{
		sim->create_part(i, x, y, PT_PHOT);
		rr = sim->rng.between(128, 355) / 127.0f;
		rrr = sim->rng.between(0, 359) * 3.14159f / 180.0f;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
	}
	return 0;
}
