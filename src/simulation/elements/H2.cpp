#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_H2()
{
	Identifier = "DEFAULT_PT_H2";
	Name = "HYGN";
	Colour = 0x5070FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.10f;
	Gravity = 0.00f;
	Diffusion = 3.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	HeatConduct = 251;
	Description = "수소: 산소와 함께 연소하여 물을 생성합니다. 고온 고압의 환경에서 핵융합합니다.";

	Properties = TYPE_GAS | PROP_PHOTPASS;

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
	auto &sd = SimulationData::CRef();
	auto &can_move = sd.can_move;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				auto rt = TYP(r);
				if (sim->pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL) // This will not work. DESL turns to fire above 5.0 pressure
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_WATR);
					sim->part_change_type(i,x,y,PT_OIL);
					return 1;
				}
				if (sim->pv[y/CELL][x/CELL] > 45.0f)
				{
					if (parts[ID(r)].temp > 2273.15)
						continue;
				}
				else
				{
					if (rt==PT_FIRE)
					{
						if(parts[ID(r)].tmp&0x02)
							parts[ID(r)].temp=3473.0f;
						else
							parts[ID(r)].temp=2473.15f;
						parts[ID(r)].tmp |= 1;
						sim->create_part(i,x,y,PT_FIRE);
						parts[i].temp += sim->rng.between(0, 99);
						parts[i].tmp |= 1;
						return 1;
					}
					else if ((rt==PT_PLSM && !(parts[ID(r)].tmp&4)) || (rt==PT_LAVA && parts[ID(r)].ctype != PT_BMTL))
					{
						sim->create_part(i,x,y,PT_FIRE);
						parts[i].temp += sim->rng.between(0, 99);
						parts[i].tmp |= 1;
						return 1;
					}
				}
			}
		}
	}
	if (parts[i].temp > 2273.15 && sim->pv[y/CELL][x/CELL] > 50.0f)
	{
		if (sim->rng.chance(1, 5))
		{
			int j;
			float temp = parts[i].temp;
			sim->create_part(i,x,y,PT_NBLE);
			parts[i].tmp = 0x1;

			j = sim->create_part(-3,x,y,PT_NEUT);
			if (j>-1)
				parts[j].temp = temp;
			if (sim->rng.chance(1, 10))
			{
				j = sim->create_part(-3,x,y,PT_ELEC);
				if (j>-1)
					parts[j].temp = temp;
			}
			j = sim->create_part(-3,x,y,PT_PHOT);
			if (j>-1)
			{
				parts[j].ctype = 0x7C0000;
				parts[j].temp = temp;
				parts[j].tmp = 0x1;
			}
			auto rx = x + sim->rng.between(-1, 1), ry = y + sim->rng.between(-1, 1), rt = TYP(pmap[ry][rx]);
			if (can_move[PT_PLSM][rt] || rt == PT_H2)
			{
				j = sim->create_part(-3,rx,ry,PT_PLSM);
				if (j>-1)
				{
					parts[j].temp = temp;
					parts[j].tmp |= 4;
				}
			}
			parts[i].temp = temp + sim->rng.between(750, 1249);
			sim->pv[y/CELL][x/CELL] += 30;
			return 1;
		}
	}
	return 0;
}
