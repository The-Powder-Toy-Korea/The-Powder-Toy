#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_THDR()
{
	Identifier = "DEFAULT_PT_THDR";
	Name = "THDR";
	Colour = 0xFFFFA0_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.0f;
	Loss = 0.30f;
	Collision = -0.99f;
	Gravity = 0.6f;
	Diffusion = 0.62f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 1;

	DefaultProperties.temp = 9000.0f + 273.15f;
	HeatConduct = 1;
	Description = "천둥: 매우 뜨거우며, 대부분의 소재에 손상을 입히고, 금속으로 전류를 방출합니다.";

	Properties = TYPE_PART;

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
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	bool kill=false;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				auto rt = TYP(r);
				if ((elements[TYP(r)].Properties&PROP_CONDUCTS) && parts[ID(r)].life==0 && !(rt==PT_WATR||rt==PT_SLTW) && parts[ID(r)].ctype!=PT_SPRK)
				{
					parts[ID(r)].ctype = parts[ID(r)].type;
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
					parts[ID(r)].life = 4;
					kill=true;
				}
				else if (rt!=PT_CLNE&&rt!=PT_THDR&&rt!=PT_SPRK&&rt!=PT_DMND&&rt!=PT_FIRE)
				{
					sim->pv[y/CELL][x/CELL] += 100.0f;
					if (sim->legacy_enable && sim->rng.chance(1, 200))
					{
						parts[i].life = sim->rng.between(120, 169);
						sim->part_change_type(i,x,y,PT_FIRE);
					}
					else
						kill=true;
				}
			}
		}
	}
	if (kill) {
		sim->kill_part(i);
		return 1;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 160;
	*fireg = 192;
	*fireb = 255;
	*firer = 144;
	*pixel_mode |= FIRE_ADD;
	return 1;
}
