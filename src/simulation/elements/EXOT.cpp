#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_EXOT()
{
	Identifier = "DEFAULT_PT_EXOT";
	Name = "EXOT";
	Colour = 0x247BFE_rgb;
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
	HotAir = 0.0003f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 2;

	Weight = 46;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 250;
	Description = "이종 물질: 다량의 전자에 노출되면 폭발합니다. 다른 많은 이상한 반응이 있습니다.";

	Properties = TYPE_LIQUID|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 1000;
	DefaultProperties.tmp = 244;

	Update = &update;
	Graphics = &graphics;
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
				auto rt = TYP(r);
				if (rt == PT_WARP)
				{
					if (parts[ID(r)].tmp2>2000 && sim->rng.chance(1, 100))
					{
						parts[i].tmp2 += 100;
					}
				}
				else if (rt == PT_EXOT)
				{
					if (parts[ID(r)].ctype == PT_PROT)
						parts[i].ctype = PT_PROT;
					if (parts[ID(r)].life == 1500 && sim->rng.chance(1, 1000))
						parts[i].life = 1500;
				}
				else if (rt == PT_LAVA)
				{
					//turn molten TTAN or molten GOLD to molten VIBR
					if (parts[ID(r)].ctype == PT_TTAN || parts[ID(r)].ctype == PT_GOLD)
					{
						if (sim->rng.chance(1, 10))
						{
							parts[ID(r)].ctype = PT_VIBR;
							sim->kill_part(i);
							return 1;
						}
					}
					//molten VIBR will kill the leftover EXOT though, so the VIBR isn't killed later
					else if (parts[ID(r)].ctype == PT_VIBR)
					{
						if (sim->rng.chance(1, 1000))
						{
							sim->kill_part(i);
							return 1;
						}
					}
				}
				if (parts[i].tmp > 245 && parts[i].life > 1337)
					if (rt!=PT_EXOT && rt!=PT_BREC && rt!=PT_DMND && rt!=PT_CLNE && rt!=PT_PRTI && rt!=PT_PRTO && rt!=PT_PCLN && rt!=PT_VOID && rt!=PT_NBHL && rt!=PT_WARP)
					{
						if (sim->create_part(i, x, y, rt) != -1)
						{
							return 1;
						}
					}
			}
		}
	}

	parts[i].tmp--;
	parts[i].tmp2--;
	//reset tmp every 250 frames, gives EXOT it's slow flashing effect
	if (parts[i].tmp < 1 || parts[i].tmp > 250)
		parts[i].tmp = 250;

	if (parts[i].tmp2 < 1)
		parts[i].tmp2 = 1;
	else if (parts[i].tmp2 > 6000)
	{
		parts[i].tmp2 = 10000;
		if (parts[i].life < 1001)
		{
			sim->part_change_type(i, x, y, PT_WARP);
			return 1;
		}
	}
	else if(parts[i].life < 1001)
		sim->pv[y/CELL][x/CELL] += (parts[i].tmp2*CFDS)/160000;

	if (sim->pv[y/CELL][x/CELL]>200 && parts[i].temp>9000 && parts[i].tmp2>200)
	{
		parts[i].tmp2 = 6000;
		sim->part_change_type(i, x, y, PT_WARP);
		return 1;
	}
	if (parts[i].tmp2 > 100)
	{
		for (auto trade = 0; trade < 9; trade++)
		{
			auto rx = sim->rng.between(-2, 2);
			auto ry = sim->rng.between(-2, 2);
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_EXOT && (parts[i].tmp2 > parts[ID(r)].tmp2) && parts[ID(r)].tmp2 >= 0) //diffusion
				{
					auto tym = parts[i].tmp2 - parts[ID(r)].tmp2;
					if (tym == 1)
					{
						parts[ID(r)].tmp2++;
						parts[i].tmp2--;
						break;
					}
					if (tym > 0)
					{
						parts[ID(r)].tmp2 += tym/2;
						parts[i].tmp2 -= tym/2;
						break;
					}
				}
			}
		}
	}
	if (parts[i].ctype == PT_PROT)
	{
		if (parts[i].temp < 50.0f)
		{
			if (sim->create_part(i, x, y, PT_CFLM) != -1) // I don't see how this could fail but whatever
			{
				return 1;
			}
		}
		else
			parts[i].temp -= 1.0f;
	}
	else if (parts[i].temp < 273.15f)
	{
		parts[i].vx = 0;
		parts[i].vy = 0;
		sim->pv[y/CELL][x/CELL] -= 0.01f;
		parts[i].tmp--;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	auto q = cpart->temp;
	auto b = cpart->tmp;
	auto c = cpart->tmp2;
	if (cpart->life < 1001)
	{
		if (gfctx.rng.chance(cpart->tmp2 - 1, 1000))
		{
			float frequency = 0.04045f;
			*colr = int(sin(frequency*c + 4) * 127 + 150);
			*colg = int(sin(frequency*c + 6) * 127 + 150);
			*colb = int(sin(frequency*c + 8) * 127 + 150);

			*firea = 100;
			*firer = 0;
			*fireg = 0;
			*fireb = 0;

			*pixel_mode |= PMODE_FLAT;
			*pixel_mode |= PMODE_FLARE;
		}
		else
		{
			float frequency = 0.00045f;
			*colr = int(sin(frequency*q + 4) * 127 + (b/1.7));
			*colg = int(sin(frequency*q + 6) * 127 + (b/1.7));
			*colb = int(sin(frequency*q + 8) * 127 + (b/1.7));
			*cola = cpart->tmp / 6;

			*firea = *cola;
			*firer = *colr;
			*fireg = *colg;
			*fireb = *colb;

			*pixel_mode |= FIRE_ADD;
			*pixel_mode |= PMODE_BLUR;
		}
	}
	else
	{
		float frequency = 0.01300f;
		*colr = int(sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*colg = int(sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*colb = int(sin(frequency*q + 6.00) * 127 + ((b/2.9) + 80));
		*cola = cpart->tmp / 6;
		*firea = *cola;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*pixel_mode |= FIRE_ADD;
		*pixel_mode |= PMODE_BLUR;
	}
	return 0;
}
