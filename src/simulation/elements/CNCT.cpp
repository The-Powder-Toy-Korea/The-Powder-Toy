#include "simulation/ElementCommon.h"

void Element::Element_CNCT()
{
	Identifier = "DEFAULT_PT_CNCT";
	Name = "CNCT";
	Colour = 0xC0C0C0_rgb;
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

	Weight = 55;

	HeatConduct = 100;
	Description = "콘크리트: ROCK 또는 스스로의 위에서 쌓이지만, 압력을 받으면 무너집니다.";

	Properties = TYPE_PART|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1123.0f;
	HighTemperatureTransition = PT_LAVA;
}
