#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ISOZ()
{
	Identifier = "DEFAULT_PT_ISOZ";
	Name = "ISOZ";
	Colour = 0xAA30D0_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 24;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "동위원소-Z: 광자에 닿거나 음압 상태이면 광자를 방출하며 붕괴합니다.";

	Properties = TYPE_LIQUID | PROP_NEUTPENETRATE | PROP_PHOTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 160.0f;
	LowTemperatureTransition = PT_ISZS;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

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
