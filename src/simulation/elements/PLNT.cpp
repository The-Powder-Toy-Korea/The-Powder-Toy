#include "simulation/ElementCommon.h"
#include <algorithm>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PLNT()
{
	Identifier = "DEFAULT_PT_PLNT";
	Name = "PLNT";
	Colour = 0x0CAC00_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;
	PhotonReflectWavelengths = 0x0007C000;

	Weight = 100;

	HeatConduct = 65;
	Description = "식물: 물을 흡수하여 자랍니다.";

	Properties = TYPE_SOLID|PROP_NEUTPENETRATE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				switch (TYP(r))
				{
				case PT_WATR:
					if (sim->rng.chance(1, 50))
					{
						auto np = sim->create_part(ID(r),x+rx,y+ry,PT_PLNT);
						if (np<0) continue;
						parts[np].life = 0;
					}
					break;
				case PT_LAVA:
					if (sim->rng.chance(1, 50))
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						parts[i].life = 4;
					}
					break;
				case PT_SMKE:
				case PT_CO2:
					if (sim->rng.chance(1, 50))
					{
						sim->kill_part(ID(r));
						parts[i].life = sim->rng.between(60, 119);
					}
					break;
				case PT_WOOD:
					{
						auto rndstore = sim->rng.gen();
						if (surround_space && !(rndstore%4) && parts[i].tmp==1)
						{
							rndstore >>= 3;
							int nnx = (rndstore%3) -1;
							rndstore >>= 2;
							int nny = (rndstore%3) -1;
							if (nnx || nny)
							{
								if (pmap[y+ry+nny][x+rx+nnx])
									continue;
								auto np = sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_VINE);
								if (np<0) continue;
								parts[np].temp = parts[i].temp;
							}
						}
					}
					break;
				default:
					continue;
				}
			}
		}
	}
	if (parts[i].life==2)
	{
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						sim->create_part(-1,x+rx,y+ry,PT_O2);
				}
			}
		}
		parts[i].life = 0;
	}
	if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = std::max((float)cpart->tmp2, cpart->temp);
	if (maxtemp > 300)
	{
		*colr += (int)restrict_flt((maxtemp-300)/5,0,58);
		*colg -= (int)restrict_flt((maxtemp-300)/2,0,102);
		*colb += (int)restrict_flt((maxtemp-300)/5,0,70);
	}
	if (maxtemp < 273)
	{
		*colg += (int)restrict_flt((273-maxtemp)/4,0,255);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,255);
	}
	return 0;
}
