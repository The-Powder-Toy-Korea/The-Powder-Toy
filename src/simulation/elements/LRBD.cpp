#include "simulation/ElementCommon.h"

void Element::Element_LRBD()
{
	Identifier = "DEFAULT_PT_LRBD";
	Name = "LRBD";
	Colour = 0xAAAAAA_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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

	Flammable = 1000;
	Explosive = 1;
	Meltable = 0;
	Hardness = 2;

	Weight = 45;

	DefaultProperties.temp = R_TEMP + 45.0f + 273.15f;
	HeatConduct = 170;
	Description = "액체 루비듐.";

	Properties = TYPE_LIQUID|PROP_CONDUCTS|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 311.0f;
	LowTemperatureTransition = PT_RBDM;
	HighTemperature = 961.0f;
	HighTemperatureTransition = PT_FIRE;
}
