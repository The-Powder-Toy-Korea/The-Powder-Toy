#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_HSWC()
{
	Identifier = "DEFAULT_PT_HSWC";
	Name = "HSWC";
	Colour = 0x3B0A0A_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "열 스위치: 활성화한 경우에만 열을 전도합니다.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		bool deserializeTemp = parts[i].tmp == 1;
		for (auto rx = -2; rx <= 2; rx++)
		{
			for (auto ry = -2; ry <= 2; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						r = sim->photons[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r) == PT_HSWC)
					{
						if (parts[ID(r)].life<10&&parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[ID(r)].life==0)
							parts[ID(r)].life = 10;
					}
					if (deserializeTemp && TYP(r) == PT_FILT)
					{
						if (rx >= -1 && rx <= 1 && ry >= -1 && ry <= 1)
						{
							int newTemp = parts[ID(r)].ctype - 0x10000000;
							if (newTemp >= MIN_TEMP && newTemp <= MAX_TEMP)
								parts[i].temp = float(parts[ID(r)].ctype - 0x10000000);
						}
					}
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*19);
	*colr += lifemod;
	return 0;
}
