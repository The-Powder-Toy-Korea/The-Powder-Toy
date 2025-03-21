#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_FRME()
{
	Identifier = "DEFAULT_PT_FRME";
	Name = "FRME";
	Colour = 0x999988_rgb;
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
	Hardness = 0;

	Weight = 100;

	HeatConduct = 0;
	Description = "프레임: 피스톤과 함께 사용하여 많은 입자를 밀어낼 수 있습니다.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
	{
		*colr += 30;
		*colg += 30;
		*colb += 30;
	}
	return 0;
}
