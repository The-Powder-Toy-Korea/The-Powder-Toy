#include "simulation/ElementCommon.h"
#include "QRTZ.h"

static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_PQRT()
{
	Identifier = "DEFAULT_PT_PQRT";
	Name = "PQRT";
	Colour = 0x88BBBB_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.27f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 90;

	HeatConduct = 3;
	Description = "석영 가루: 깨진 형태의 석영입니다.";

	Properties = TYPE_PART | PROP_PHOTPASS | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2573.15f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_QRTZ_update;
	Graphics = &Element_QRTZ_graphics;
	Create = &create;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp2 = sim->rng.between(0, 10);
}
