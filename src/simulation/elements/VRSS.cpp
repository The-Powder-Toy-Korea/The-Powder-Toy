#include "simulation/ElementCommon.h"
#include "VIRS.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_VRSS()
{
	Identifier = "DEFAULT_PT_VRSS";
	Name = "VRSS";
	Colour = 0xD408CD_rgb;
	MenuVisible = 0;
	MenuSection = SC_SOLIDS;
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
	Hardness = 1;

	Weight = 100;

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 251;
	Description = "고체 바이러스: 닿는 모든 것을 자기 자신으로 바꿉니다.";

	Properties = TYPE_SOLID|PROP_DEADLY;
	CarriesTypeIn = 1U << FIELD_TMP2;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 305.0f;
	HighTemperatureTransition = PT_VIRS;

	DefaultProperties.tmp4 = 250;

	Update = &Element_VIRS_update;
	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= NO_DECO;
	return 1;
}
