#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CO2()
{
	Identifier = "DEFAULT_PT_CO2";
	Name = "CO2";
	Colour = 0x666666_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.1f;
	Diffusion = 1.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	HeatConduct = 88;
	Description = "이산화 탄소: 무거운 기체로, 바닥으로 가라앉습니다. 물에 녹으며 차가워지면 드라이아이스로 변합니다.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 194.65f;
	LowTemperatureTransition = PT_DRIC;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
				{
					if (parts[i].ctype==5 && sim->rng.chance(1, 2000))
					{
						if (sim->create_part(-1, x+rx, y+ry, PT_WATR)>=0)
							parts[i].ctype = 0;
					}
					continue;
				}
				if (TYP(r)==PT_FIRE)
				{
					sim->kill_part(ID(r));
					if (sim->rng.chance(1, 30))
					{
						sim->kill_part(i);
						return 1;
					}
				}
				else if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && sim->rng.chance(1, 50))
				{
					sim->part_change_type(ID(r), x+rx, y+ry, PT_CBNW);
					if (parts[i].ctype==5) //conserve number of water particles - ctype=5 means this CO2 hasn't released the water particle from BUBW yet
					{
						sim->create_part(i, x, y, PT_WATR);
						return 0;
					}
					else
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
		}
	}
	if (parts[i].temp > 9773.15 && sim->pv[y/CELL][x/CELL] > 200.0f)
	{
		if (sim->rng.chance(1, 5))
		{
			int j;
			sim->create_part(i,x,y,PT_O2);
			j = sim->create_part(-3,x,y,PT_NEUT);
			if (j != -1)
				parts[j].temp = MAX_TEMP;
			if (sim->rng.chance(1, 50))
			{
				j = sim->create_part(-3,x,y,PT_ELEC);
				if (j != -1)
					parts[j].temp = MAX_TEMP;
			}
			parts[i].temp = MAX_TEMP;
			sim->pv[y/CELL][x/CELL] += 100;
		}
	}
	return 0;
}
