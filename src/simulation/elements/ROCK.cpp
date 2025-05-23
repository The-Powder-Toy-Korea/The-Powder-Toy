#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_ROCK()
{
	Identifier = "DEFAULT_PT_ROCK";
	Name = "ROCK";
	Colour = 0x727272_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 5;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 200;
	Description = "암석: 단단하며, 다양한 요소로 녹습니다.";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 120;
	HighPressureTransition = PT_STNE;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1943.15f;
	HighTemperatureTransition = PT_LAVA;

	Graphics = &graphics;
	Create = &create;
}


static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp2 - 7) * 6; // Randomized color noise based on tmp2
	*colr += z;
	*colg += z;
	*colb += z;

	if (cpart->temp >= 810.15) // Glows when hot, right before melting becomes bright
	{
		*pixel_mode |= FIRE_ADD;

		*firea = int(((cpart->temp)-810.15)/45);
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = sim->rng.between(0, 10);
}
