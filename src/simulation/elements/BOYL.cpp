#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BOYL()
{
	Identifier = "DEFAULT_PT_BOYL";
	Name = "BOYL";
	Colour = 0x0A3200_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.18f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 42;
	Description = "보일 기체: 압력이 변하는 기체입니다. 가열하면 팽창합니다.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	float limit = parts[i].temp / 100;
	if (sim->pv[y / CELL][x / CELL] < limit)
		sim->pv[y / CELL][x / CELL] += 0.001f*(limit - sim->pv[y / CELL][x / CELL]);
	if (sim->pv[y / CELL + 1][x / CELL] < limit)
		sim->pv[y / CELL + 1][x / CELL] += 0.001f*(limit - sim->pv[y / CELL + 1][x / CELL]);
	if (sim->pv[y / CELL - 1][x / CELL] < limit)
		sim->pv[y / CELL - 1][x / CELL] += 0.001f*(limit - sim->pv[y / CELL - 1][x / CELL]);

	sim->pv[y / CELL][x / CELL + 1]	+= 0.001f*(limit - sim->pv[y / CELL][x / CELL + 1]);
	sim->pv[y / CELL + 1][x / CELL + 1] += 0.001f*(limit - sim->pv[y / CELL + 1][x / CELL + 1]);
	sim->pv[y / CELL][x / CELL - 1]	+= 0.001f*(limit - sim->pv[y / CELL][x / CELL - 1]);
	sim->pv[y / CELL - 1][x / CELL - 1] += 0.001f*(limit - sim->pv[y / CELL - 1][x / CELL - 1]);

	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_WATR)
				{
					if (sim->rng.chance(1, 30))
						sim->part_change_type(ID(r),x+rx,y+ry,PT_FOG);
				}
				else if (TYP(r)==PT_O2)
				{
					if (sim->rng.chance(1, 9))
					{
						sim->kill_part(ID(r));
						sim->part_change_type(i,x,y,PT_WATR);
						sim->pv[y/CELL][x/CELL] += 4.0;
					}
				}
			}
		}
	}
	return 0;
}
