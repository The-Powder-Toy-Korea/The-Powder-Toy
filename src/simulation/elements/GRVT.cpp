#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_GRVT()
{
	Identifier = "DEFAULT_PT_GRVT";
	Name = "GRVT";
	Colour = 0x00EE76_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = -1;

	HeatConduct = 61;
	Description = "중력자: 뉴턴 중력을 발생시킵니다.";

	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.tmp = 7;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	//at higher tmps they just go completely insane
	if (parts[i].tmp >= 100)
		parts[i].tmp = 100;
	if (parts[i].tmp <= -100)
		parts[i].tmp = -100;

	int under = pmap[y][x];
	int utype = TYP(under);

	//Randomly kill GRVT inside RSSS
	if((utype == PT_RSSS) && sim->rng.chance(1, 5))
	{

		sim->kill_part(i);
		return 1;
	}
	sim->gravIn.mass[Vec2{ x, y } / CELL] = 0.2f * parts[i].tmp;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 5;
	*firer = 0;
	*fireg = 250;
	*fireb = 170;

	*pixel_mode |= FIRE_BLEND;
	return 1;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	float a = sim->rng.between(0, 359) * 3.14159f / 180.0f;
	sim->parts[i].life = 250 + sim->rng.between(0, 199);
	sim->parts[i].vx = 2.0f*cosf(a);
	sim->parts[i].vy = 2.0f*sinf(a);
}
