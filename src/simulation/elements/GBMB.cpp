#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GBMB()
{
	Identifier = "DEFAULT_PT_GBMB";
	Name = "GBMB";
	Colour = 0x1144BB_rgb;
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "중력 폭탄: 처음 닿는 물체에 달라붙은 다음 강한 중력으로 밀어냅니다.";

	Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life<=0)
	{
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				auto r = pmap[y+ry][x+rx];
				if(!r)
					continue;
				if(TYP(r)!=PT_BOMB && TYP(r)!=PT_GBMB &&
				   TYP(r)!=PT_CLNE && TYP(r)!=PT_PCLN &&
				   TYP(r)!=PT_DMND)
				{
					parts[i].life=60;
					break;
				}
			}
		}
	}
	if (parts[i].life > 20)
	{
		sim->gravIn.mass[Vec2{ x, y } / CELL] = 20;
	}
	else if (parts[i].life >= 1)
	{
		sim->gravIn.mass[Vec2{ x, y } / CELL] = -80;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life <= 0) {
		*pixel_mode |= PMODE_FLARE;
	}
	else
	{
		*pixel_mode |= PMODE_SPARK;
	}
	return 0;
}
