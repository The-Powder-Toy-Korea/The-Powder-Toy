#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_CBNW()
{
	Identifier = "DEFAULT_PT_CBNW";
	Name = "BUBW";
	Colour = 0x2030D0_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
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
	Hardness = 20;

	Weight = 30;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	LatentHeat = 7500;
	Description = "탄산수: 김이 빠지며 천천히 이산화 탄소를 방출합니다.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 273.15f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 373.0f;
	HighTemperatureTransition = PT_WTRV;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (sim->pv[y/CELL][x/CELL]<=3)
	{
		if (sim->pv[y/CELL][x/CELL] <= -0.5 || sim->rng.chance(1, 4000))
		{
			sim->part_change_type(i,x,y,PT_CO2);
			parts[i].ctype = 5;
			sim->pv[y/CELL][x/CELL] += 0.5f;
		}
	}
	if (parts[i].tmp2!=20) {
		parts[i].tmp2 -= (parts[i].tmp2>20)?1:-1;
	}
	else if (sim->rng.chance(1, 200))
	{
		parts[i].tmp2 = sim->rng.between(0, 39);
	}

	if(parts[i].tmp>0)
	{
		//Explode
		if(parts[i].tmp==1 && sim->rng.chance(3, 4))
		{
			sim->part_change_type(i,x,y,PT_CO2);
			parts[i].ctype = 5;
			sim->pv[y/CELL][x/CELL] += 0.2f;
		}
		parts[i].tmp--;
	}
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((elements[TYP(r)].Properties&TYPE_PART) && parts[i].tmp == 0 && sim->rng.chance(1, 83))
				{
					//Start explode
					parts[i].tmp = sim->rng.between(0, 24);
				}
				else if((elements[TYP(r)].Properties&TYPE_SOLID) && TYP(r)!=PT_DMND && TYP(r)!=PT_GLAS && parts[i].tmp == 0 && sim->rng.chance(int(2 - sim->pv[y/CELL][x/CELL]), 6667))
				{
					sim->part_change_type(i,x,y,PT_CO2);
					parts[i].ctype = 5;
					sim->pv[y/CELL][x/CELL] += 0.2f;
				}
				if (TYP(r)==PT_CBNW)
				{
					if(!parts[i].tmp)
					{
						if (parts[ID(r)].tmp)
						{
							parts[i].tmp = parts[ID(r)].tmp;
							if((ID(r))>i) //If the other particle hasn't been life updated
								parts[i].tmp--;
						}
					}
					else if(!parts[ID(r)].tmp)
					{
						parts[ID(r)].tmp = parts[i].tmp;
						if((ID(r))>i) //If the other particle hasn't been life updated
							parts[ID(r)].tmp++;
					}
				}
				else if (TYP(r)==PT_RBDM||TYP(r)==PT_LRBD)
				{
					if ((sim->legacy_enable||parts[i].temp>(273.15f+12.0f)) && sim->rng.chance(1, 166))
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						parts[i].life = 4;
						parts[i].ctype = PT_WATR;
					}
				}
				else if (TYP(r)==PT_FIRE && parts[ID(r)].ctype!=PT_WATR){
					sim->kill_part(ID(r));
					if (sim->rng.chance(1, 50))
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = cpart->tmp2 - 20;//speckles!
	*colr += z * 1;
	*colg += z * 2;
	*colb += z * 8;
	return 0;
}
