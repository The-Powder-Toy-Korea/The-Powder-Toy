#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FRAY()
{
	Identifier = "DEFAULT_PT_FRAY";
	Name = "FRAY";
	Colour = 0x00BBFF_rgb;
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = 20.0f + 273.15f;
	HeatConduct = 0;
	Description = "힘 선 방출기: 온도에 따라 개체를 밀거나 당깁니다. ARAY처럼 사용합니다.";

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
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int curlen;
	if (parts[i].tmp > 0)
		curlen = parts[i].tmp;
	else
		curlen = 10;
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SPRK)
				{
					for (auto nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1, len = 0; ; nyy+=nyi, nxx+=nxi, len++)
					{
						if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0) || len>curlen)
						{
							break;
						}
						r = pmap[y+nyi+nyy][x+nxi+nxx];
						if (!r)
							r = sim->photons[y+nyi+nyy][x+nxi+nxx];
						if (r && !(elements[TYP(r)].Properties & TYPE_SOLID)){
							parts[ID(r)].vx += nxi*((parts[i].temp-273.15f)/10.0f);
							parts[ID(r)].vy += nyi*((parts[i].temp-273.15f)/10.0f);
						}
					}
				}
			}
		}
	}
	return 0;
}
