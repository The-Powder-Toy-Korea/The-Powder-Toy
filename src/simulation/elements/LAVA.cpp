#include "simulation/ElementCommon.h"
#include "FIRE.h"

static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LAVA()
{
	Identifier = "DEFAULT_PT_LAVA";
	Name = "LAVA";
	Colour = 0xE05010_rgb;
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
	HotAir = 0.0003f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;
	PhotonReflectWavelengths = 0x3FF00000;

	Weight = 45;

	DefaultProperties.temp = R_TEMP + 1500.0f + 273.15f;
	HeatConduct = 60;
	Description = "용암: 가연성 소재를 점화합니다. 금속 및 기타 소재가 녹을 때 생성되며, 차가워지면 굳습니다.";

	Properties = TYPE_LIQUID|PROP_LIFE_DEC;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = MAX_TEMP;// check for lava solidification at all temperatures
	LowTemperatureTransition = ST;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_FIRE_update;
	Graphics = &graphics;
	Create = &create;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = cpart->life * 2 + 0xE0;
	*colg = cpart->life * 1 + 0x50;
	*colb = cpart->life / 2 + 0x10;
	if (*colr>255) *colr = 255;
	if (*colg>192) *colg = 192;
	if (*colb>128) *colb = 128;
	*firea = 40;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	*pixel_mode |= FIRE_ADD;
	*pixel_mode |= PMODE_BLUR;
	//Returning 0 means dynamic, do not cache
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = sim->rng.between(240, 359);
}
