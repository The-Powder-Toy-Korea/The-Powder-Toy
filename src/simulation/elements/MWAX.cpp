#include "simulation/ElementCommon.h"

void Element::Element_MWAX()
{
	Identifier = "DEFAULT_PT_MWAX";
	Name = "MWAX";
	Colour = 0xE0E0AA_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000001f* CFDS;
	Falldown = 2;

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 25;

	DefaultProperties.temp = R_TEMP + 28.0f + 273.15f;
	HeatConduct = 44;
	Description = "녹은 왁스: 45 °C에서 왁스로 굳어집니다.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 318.0f;
	LowTemperatureTransition = PT_WAX;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_FIRE;
}
