#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SLTW()
{
	Identifier = "DEFAULT_PT_SLTW";
	Name = "SLTW";
	Colour = 0x4050F0_rgb;
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

	Weight = 35;

	HeatConduct = 75;
	LatentHeat = 7500;
	Description = "소금물: 전기 전도성이 있으며 잘 얼지 않습니다.";

	Properties = TYPE_LIQUID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_NEUTPENETRATE | PROP_PHOTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 252.05f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 383.0f;
	HighTemperatureTransition = ST;

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
				switch (TYP(r))
				{
				case PT_SALT:
					if (sim->rng.chance(1, 2000))
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SLTW);
					break;
				case PT_PLNT:
					if (sim->rng.chance(1, 40))
						sim->kill_part(ID(r));
					break;
				case PT_RBDM:
				case PT_LRBD:
					if ((sim->legacy_enable||parts[i].temp>(273.15f+12.0f)) && sim->rng.chance(1, 100))
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						parts[i].life = 4;
						parts[i].ctype = PT_WATR;
					}
					break;
				case PT_FIRE:
					if (parts[ID(r)].ctype!=PT_WATR)
					{
						sim->kill_part(ID(r));
						if (sim->rng.chance(1, 30))
						{
							sim->kill_part(i);
							return 1;
						}
					}
					break;
				case PT_NONE:
					break;
				default:
					continue;
				}
			}
		}
	}
	return 0;
}
