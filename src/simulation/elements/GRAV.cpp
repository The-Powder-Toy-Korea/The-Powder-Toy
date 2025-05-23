#include "simulation/ElementCommon.h"
#include <algorithm>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GRAV()
{
	Identifier = "DEFAULT_PT_GRAV";
	Name = "GRAV";
	Colour = 0x202020_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 85;

	HeatConduct = 70;
	Description = "중력 가루: 매우 가벼운 먼지입니다. 속도에 따라 색상이 변화합니다.";

	Properties = TYPE_PART | PROP_LIFE_DEC;

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
	if (parts[i].vx*parts[i].vx + parts[i].vy*parts[i].vy >= 0.1f && sim->rng.chance(1, 512))
	{
		if (!parts[i].life)
			parts[i].life = 48;
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int GRAV_R, GRAV_B, GRAV_G, GRAV_R2, GRAV_B2, GRAV_G2;

	GRAV_R = std::abs((gfctx.sim->currentTick%120)-60);
	GRAV_G = std::abs(((gfctx.sim->currentTick+60)%120)-60);
	GRAV_B = std::abs(((gfctx.sim->currentTick+120)%120)-60);
	GRAV_R2 = std::abs((gfctx.sim->currentTick%60)-30);
	GRAV_G2 = std::abs(((gfctx.sim->currentTick+30)%60)-30);
	GRAV_B2 = std::abs(((gfctx.sim->currentTick+60)%60)-30);


	*colr = 20;
	*colg = 20;
	*colb = 20;
	if (cpart->vx>0)
	{
		*colr += int((cpart->vx)*GRAV_R);
		*colg += int((cpart->vx)*GRAV_G);
		*colb += int((cpart->vx)*GRAV_B);
	}
	if (cpart->vy>0)
	{
		*colr += int((cpart->vy)*GRAV_G);
		*colg += int((cpart->vy)*GRAV_B);
		*colb += int((cpart->vy)*GRAV_R);

	}
	if (cpart->vx<0)
	{
		*colr -= int((cpart->vx)*GRAV_B);
		*colg -= int((cpart->vx)*GRAV_R);
		*colb -= int((cpart->vx)*GRAV_G);

	}
	if (cpart->vy<0)
	{
		*colr -= int((cpart->vy)*GRAV_R2);
		*colg -= int((cpart->vy)*GRAV_G2);
		*colb -= int((cpart->vy)*GRAV_B2);
	}

	if (cpart->life)
	{
		*pixel_mode = FIRE_ADD | PMODE_ADD | PMODE_GLOW | PMODE_FLARE;
		*firer = std::min(*colr * 3, 255);
		*fireg = std::min(*colg * 3, 255);
		*fireb = std::min(*colb * 3, 255);
		*firea = (cpart->life+15)*4;
		*cola = (cpart->life+15)*4;
	}
	else
		*pixel_mode = PMODE_ADD;

	return 0;
}
