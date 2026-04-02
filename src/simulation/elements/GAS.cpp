#include "simulation/ElementCommon.h"

void Element::Element_GAS()
{
	Identifier = "DEFAULT_PT_GAS";
	Name = "GAS";
	Colour = 0xE0FF20_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 1.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 0.75f;
	HotAir = 0.001f	* CFDS;
	Falldown = 0;

	Flammable = 600;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 1;

	DefaultProperties.temp = R_TEMP + 2.0f + 273.15f;
	HeatConduct = 42;
	Description = "가스: 빠르게 확산하며 가연성입니다. 압력을 받으면 기름으로 액화합니다.";

	Properties = TYPE_GAS | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 6.0f;
	HighPressureTransition = PT_OIL;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;
}
