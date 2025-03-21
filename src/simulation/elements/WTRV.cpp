#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_WTRV()
{
	Identifier = "DEFAULT_PT_WTRV";
	Name = "WTRV";
	Colour = 0xA0A0FF_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = -0.1f;
	Diffusion = 0.75f;
	HotAir = 0.0003f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 4;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 100.0f + 273.15f;
	HeatConduct = 48;
	Description = "수증기: 뜨거운 물에서 생성됩니다.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 371.0f;
	LowTemperatureTransition = ST;
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
					continue;
				if ((TYP(r)==PT_RBDM||TYP(r)==PT_LRBD) && !sim->legacy_enable && parts[i].temp>(273.15f+12.0f) && sim->rng.chance(1, 100))
				{
					sim->part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
					parts[i].ctype = PT_WATR;
				}
			}
		}
	}
	if(parts[i].temp>1273&&parts[i].ctype==PT_FIRE)
		parts[i].temp-=parts[i].temp/1000;
	return 0;
}
