#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GOLD()
{
	Identifier = "DEFAULT_PT_GOLD";
	Name = "GOLD";
	Colour = 0xDCAD2C_rgb;
	MenuVisible = 1;
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
	Meltable = 1;
	Hardness = 0;
	PhotonReflectWavelengths = 0x3C038100;

	Weight = 100;

	HeatConduct = 251;
	Description = "금: 내식성 금속으로 철의 부식을 방지합니다. 우수한 도체입니다.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_HOT_GLOW|PROP_LIFE_DEC|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1337.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	static int checkCoordsX[] = { -4, 4, 0, 0 };
	static int checkCoordsY[] = { 0, 0, -4, 4 };
	//Find nearby rusted iron (BMTL with tmp 1+)
	for(int j = 0; j < 8; j++)
	{
		auto rndstore = sim->rng.gen();
		auto rx = (rndstore % 9)-4;
		rndstore >>= 4;
		auto ry = (rndstore % 9)-4;
		if ((!rx != !ry)) {
			auto r = pmap[y+ry][x+rx];
			if(!r) continue;
			if(TYP(r)==PT_BMTL && parts[ID(r)].tmp)
			{
				parts[ID(r)].tmp = 0;
				sim->part_change_type(ID(r), x+rx, y+ry, PT_IRON);
			}
		}
	}
	//Find sparks
	if(!parts[i].life)
	{
		for(int j = 0; j < 4; j++)
		{
			auto rx = checkCoordsX[j];
			auto ry = checkCoordsY[j];
			auto r = pmap[y+ry][x+rx];
			if(!r) continue;
			if(TYP(r)==PT_SPRK && parts[ID(r)].life && parts[ID(r)].life<4)
			{
				sim->part_change_type(i, x, y, PT_SPRK);
				parts[i].life = 4;
				parts[i].ctype = PT_GOLD;
			}
		}
	}
	if (TYP(sim->photons[y][x]) == PT_NEUT)
	{
		if (sim->rng.chance(1, 7))
		{
			sim->kill_part(ID(sim->photons[y][x]));
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int rndstore = gfctx.rng.gen();
	*colr += (rndstore % 10) - 5;
	rndstore >>= 4;
	*colg += (rndstore % 10)- 5;
	rndstore >>= 4;
	*colb += (rndstore % 10) - 5;
	return 0;
}
