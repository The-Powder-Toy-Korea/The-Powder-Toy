#include "simulation/ElementCommon.h"
#include "VIBR.h"

void Element::Element_BVBR()
{
	Identifier = "DEFAULT_PT_BVBR";
	Name = "BVBR";
	Colour = 0x005000_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.0000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 67;

	DefaultProperties.temp = 273.15f;
	HeatConduct = 164;
	Description = "비브라늄 가루: 비브라늄이 압력에 의해 부서질 때 생성됩니다.";

	Properties = TYPE_PART|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_VIBR_update;
	Graphics = &Element_VIBR_graphics;
}
