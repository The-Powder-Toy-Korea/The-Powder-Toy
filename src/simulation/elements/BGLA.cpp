#include "simulation/ElementCommon.h"

void Element::Element_BGLA()
{
	Identifier = "DEFAULT_PT_BGLA";
	Name = "BGLA";
	Colour = 0x606060_rgb;
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
	Meltable = 5;
	Hardness = 0;

	Weight = 90;

	HeatConduct = 150;
	Description = "유리 가루: 유리가 압력을 받아 깨질 때 형성되는 무거운 입자입니다. 녹을 수 있는 입자입니다.";

	Properties = TYPE_PART | PROP_NEUTPASS | PROP_PHOTPASS | PROP_HOT_GLOW | PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1973.0f;
	HighTemperatureTransition = PT_LAVA;
}
