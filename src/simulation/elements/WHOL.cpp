#include "simulation/ElementCommon.h"

void Element::Element_WHOL()
{
	Identifier = "DEFAULT_PT_WHOL";
	Name = "VENT";
	Colour = 0xEFEFEF_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.010f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	DefaultProperties.temp = R_TEMP - 16.0f + 273.15f;
	HeatConduct = 255;
	Description = "배기구: 압력을 생성하여 입자를 밀어냅니다.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
}
