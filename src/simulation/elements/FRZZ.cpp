#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FRZZ()
{
	Identifier = "DEFAULT_PT_FRZZ";
	Name = "FRZZ";
	Colour = 0xC0E0FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.90f;
	Collision = -0.1f;
	Gravity = 0.05f;
	Diffusion = 0.01f;
	HotAir = -0.00005f* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 50;

	DefaultProperties.temp = 253.15f;
	HeatConduct = 46;
	Description = "동결 가루: 녹으면 스스로 차가워지는 얼음이 됩니다. 일반 물과 섞어 사용하십시오.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 1.8f;
	HighPressureTransition = PT_SNOW;
	LowTemperature = 50.0f;
	LowTemperatureTransition = PT_ICEI;
	HighTemperature = 273.15f;
	HighTemperatureTransition = PT_FRZW;

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
					continue;
				if (TYP(r)==PT_WATR && sim->rng.chance(1, 20))
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_FRZW);
					parts[ID(r)].life = 100;
					sim->kill_part(i);
					return 1;
				}
			}
		}
	}
	return 0;
}
