#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_CLST()
{
	Identifier = "DEFAULT_PT_CLST";
	Name = "CLST";
	Colour = 0xE4A4A4_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.2f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 2;
	Hardness = 2;

	Weight = 55;

	HeatConduct = 70;
	Description = "점토 가루: 물과 섞으면 페이스트가 됩니다.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1256.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_WATR)
				{
					if (sim->rng.chance(1, 1500))
					{
						sim->create_part(i, x, y, PT_PSTS);
						sim->kill_part(ID(r));
					}
				}
				else if (TYP(r)==PT_NITR)
				{
					sim->create_part(i, x, y, PT_BANG);
					sim->create_part(ID(r), x+rx, y+ry, PT_BANG);
				}
				else if (TYP(r)==PT_CLST)
				{
					float cxy;
					if(parts[i].temp <195)
						cxy = 0.05f;
					else if(parts[i].temp <295)
						cxy = 0.015f;
					else if(parts[i].temp <350)
						cxy = 0.01f;
					else
						cxy = 0.005f;
					parts[i].vx += cxy*rx;
					parts[i].vy += cxy*ry;//These two can be set not to calculate over 350 later. They do virtually nothing over 0.005.
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int z = (cpart->tmp - 5) * 16;//speckles!
	*colr += z;
	*colg += z;
	*colb += z;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].tmp = sim->rng.between(0, 6);
}
