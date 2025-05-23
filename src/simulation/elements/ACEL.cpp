#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_ACEL()
{
	Identifier = "DEFAULT_PT_ACEL";
	Name = "ACEL";
	Colour = 0x0099CC_rgb;
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
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "가속기: 주변 요소의 속도를 높입니다.";

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
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	float multiplier;
	if (parts[i].life!=0)
	{
		auto change = parts[i].life > 1000 ? 1000 : (parts[i].life < 0 ? 0 : parts[i].life);
		multiplier = 1.0f+(change/100.0f);
	}
	else
	{
		multiplier = 1.1f;
	}
	parts[i].tmp = 0;
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (!rx != !ry)
			{
				auto r = pmap[y+ry][x+rx];
				if(!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				if(elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[ID(r)].vx *= multiplier;
					parts[ID(r)].vy *= multiplier;
					parts[i].tmp = 1;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}
