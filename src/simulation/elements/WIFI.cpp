#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_WIFI()
{
	Identifier = "DEFAULT_PT_WIFI";
	Name = "WIFI";
	Colour = 0x40A060_rgb;
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Hardness = 2;

	Weight = 100;

	HeatConduct = 0;
	Description = "무선 송수신기: 같은 온도 채널의 다른 송수신기로 전류를 전송합니다.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				// wireless[][0] - whether channel is active on this frame
				// wireless[][1] - whether channel should be active on next frame
				if (sim->wireless[parts[i].tmp][0])
				{
					if ((TYP(r)==PT_NSCN||TYP(r)==PT_PSCN||TYP(r)==PT_INWR)&&parts[ID(r)].life==0 && sim->wireless[parts[i].tmp][0])
					{
						parts[ID(r)].ctype = TYP(r);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
						parts[ID(r)].life = 4;
					}
				}
				if (TYP(r)==PT_SPRK && parts[ID(r)].ctype!=PT_NSCN && parts[ID(r)].life>=3)
				{
					sim->wireless[parts[i].tmp][1] = 1;
					sim->ISWIRE = 2;
				}
			}
		}
	}
	return 0;
}

constexpr float FREQUENCY = 0.0628f;

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int q = (int)((cpart->temp-73.15f)/100+1);
	*colr = int(sin(FREQUENCY*q + 0) * 127 + 128);
	*colg = int(sin(FREQUENCY*q + 2) * 127 + 128);
	*colb = int(sin(FREQUENCY*q + 4) * 127 + 128);
	*pixel_mode |= EFFECT_DBGLINES;
	return 0;
}
