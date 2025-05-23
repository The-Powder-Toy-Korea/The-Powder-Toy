#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_BRMT()
{
	Identifier = "DEFAULT_PT_BRMT";
	Name = "BRMT";
	Colour = 0x705060_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 90;

	HeatConduct = 211;
	Description = "메탈 가루: 철이 녹슬거나 BMTL이 압력으로 부서질 때 발생합니다.";

	Properties = TYPE_PART|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = ST;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].temp > 523.15f)//250.0f+273.15f
	{
		auto tempFactor = int(1000 - ((523.15f-parts[i].temp)*2));
		if(tempFactor < 2)
			tempFactor = 2;
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_BREC && sim->rng.chance(1, tempFactor))
					{
						if (sim->rng.chance(1, 2))
						{
							sim->create_part(ID(r), x+rx, y+ry, PT_THRM);
						}
						else
							sim->create_part(i, x, y, PT_THRM);
					}
				}
			}
		}
	}
	return 0;
}
