#include "simulation/ElementCommon.h"
#include "FIRE.h"
#include <algorithm>

static int updateLegacy(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_FIRE()
{
	Identifier = "DEFAULT_PT_FIRE";
	Name = "FIRE";
	Colour = 0xFF1000_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.9f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.20f;
	Collision = 0.0f;
	Gravity = -0.1f;
	Diffusion = 0.00f;
	HotAir = 0.001f  * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 2;

	DefaultProperties.temp = R_TEMP + 400.0f + 273.15f;
	HeatConduct = 88;
	Description = "불: 가연성 소재를 점화합니다. 공기를 가열합니다.";

	Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 2773.0f;
	HighTemperatureTransition = PT_PLSM;

	Update = &Element_FIRE_update;
	Graphics = &graphics;
	Create = &create;
}

int Element_FIRE_update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int t = parts[i].type;
	switch (t)
	{
	case PT_PLSM:
		if (parts[i].life <=1)
		{
			if (parts[i].ctype == PT_NBLE)
			{
				sim->part_change_type(i,x,y,PT_NBLE);
				parts[i].life = 0;
			}
			else if ((parts[i].tmp&0x3) == 3){
				sim->part_change_type(i,x,y,PT_WTRV);
				parts[i].life = 0;
				parts[i].ctype = PT_FIRE;
			}
		}
		break;
	case PT_FIRE:
		if (parts[i].life <=1)
		{
			if ((parts[i].tmp&0x3) == 3){
				sim->part_change_type(i,x,y,PT_WTRV);
				parts[i].life = 0;
				parts[i].ctype = PT_FIRE;
			}
			else if (parts[i].temp<625)
			{
				sim->part_change_type(i,x,y,PT_SMKE);
				parts[i].life = sim->rng.between(250, 269);
			}
		}
		break;
	case PT_LAVA: {
		float pres = sim->pv[y / CELL][x / CELL];
		if (parts[i].ctype == PT_ROCK)
		{			
			if (pres <= -9)
			{
				parts[i].ctype = PT_STNE;
				break;
			}

			if (pres >= 25 && sim->rng.chance(1, 12500))
			{
				if (pres <= 50)
				{
					if (sim->rng.chance(1, 2))
						parts[i].ctype = PT_BRMT;
					else
						parts[i].ctype = PT_CNCT;
				}
				else if (pres <= 75)
				{
					if (pres >= 73 || sim->rng.chance(1, 8))
						parts[i].ctype = PT_GOLD;
					else
						parts[i].ctype = PT_QRTZ;
				}
				else if (pres <= 100 && parts[i].temp >= 5000)
				{
					if (sim->rng.chance(1, 5)) // 1 in 5 chance IRON to TTAN
						parts[i].ctype = PT_TTAN;
					else
						parts[i].ctype = PT_IRON;
				}
				else if (parts[i].temp >= 5000 && sim->rng.chance(1, 5))
				{
					if (sim->rng.chance(1, 5))
						parts[i].ctype = PT_URAN;
					else if (sim->rng.chance(1, 5))
						parts[i].ctype = PT_PLUT;
					else
						parts[i].ctype = PT_TUNG;
				}
			}
		}
		else if ((parts[i].ctype == PT_STNE || !parts[i].ctype) && pres >= 30.0f && (parts[i].temp > elements[PT_ROCK].HighTemperature || pres < elements[PT_ROCK].HighPressure)) // Form ROCK with pressure, if it will stay molten or not immediately break
		{
			parts[i].tmp2 = sim->rng.between(0, 10); // Provide tmp2 for color noise
			parts[i].ctype = PT_ROCK;
		}
		break;
	}
	default:
		break;
	}
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

				//THRM burning
				if (rt==PT_THRM && (t==PT_FIRE || t==PT_PLSM || t==PT_LAVA))
				{
					if (sim->rng.chance(1, 500)) {
						sim->part_change_type(ID(r),x+rx,y+ry,PT_LAVA);
						parts[ID(r)].ctype = PT_BMTL;
						parts[ID(r)].temp = 3500.0f;
						sim->pv[(y+ry)/CELL][(x+rx)/CELL] += 50.0f;
					} else {
						sim->part_change_type(ID(r),x+rx,y+ry,PT_LAVA);
						parts[ID(r)].life = 400;
						parts[ID(r)].ctype = PT_THRM;
						parts[ID(r)].temp = 3500.0f;
						parts[ID(r)].tmp = 20;
					}
					continue;
				}

				if ((rt==PT_COAL) || (rt==PT_BCOL))
				{
					if ((t==PT_FIRE || t==PT_PLSM))
					{
						if (parts[ID(r)].life>100 && sim->rng.chance(1, 500))
						{
							parts[ID(r)].life = 99;
						}
					}
					else if (t==PT_LAVA)
					{
						if (parts[i].ctype == PT_IRON && sim->rng.chance(1, 500))
						{
							parts[i].ctype = PT_METL;
							sim->kill_part(ID(r));
							continue;
						}
						if ((parts[i].ctype == PT_STNE || parts[i].ctype == PT_NONE) && sim->rng.chance(1, 60))
						{
							parts[i].ctype = PT_SLCN;
							sim->kill_part(ID(r));
							continue;
						}
					}
				}

				if (t == PT_LAVA)
				{
					// LAVA(CLST) + LAVA(PQRT) + high enough temp = LAVA(CRMC) + LAVA(CRMC)
					if (parts[i].ctype == PT_QRTZ && rt == PT_LAVA && parts[ID(r)].ctype == PT_CLST)
					{
						float pres = std::max(sim->pv[y/CELL][x/CELL]*10.0f, 0.0f);
						if (parts[i].temp >= pres+elements[PT_CRMC].HighTemperature+50.0f)
						{
							parts[i].ctype = PT_CRMC;
							parts[ID(r)].ctype = PT_CRMC;
						}
					}
					else if (rt == PT_O2 && parts[i].ctype == PT_SLCN)
					{
						switch (sim->rng.between(0, 2))
						{
						case 0:
							parts[i].ctype = PT_SAND;
							break;

						case 1:
							parts[i].ctype = PT_CLST;
							// avoid creating CRMC.
							if (parts[i].temp >= elements[PT_PQRT].HighTemperature * 3)
							{
								parts[i].ctype = PT_PQRT;
							}
							break;

						case 2:
							parts[i].ctype = PT_STNE;
							break;
						}
						parts[i].tmp = 0;
						sim->kill_part(ID(r));
						continue;
					}
					else if (rt == PT_LAVA && (parts[ID(r)].ctype == PT_METL || parts[ID(r)].ctype == PT_BMTL) && parts[i].ctype == PT_SLCN)
					{
						parts[i].tmp = 0;
						parts[i].ctype = PT_NSCN;
						parts[ID(r)].ctype = PT_PSCN;
					}
					else if (rt == PT_HEAC && parts[i].ctype == PT_HEAC)
					{
						if (parts[ID(r)].temp > elements[PT_HEAC].HighTemperature)
						{
							sim->part_change_type(ID(r), x+rx, y+ry, PT_LAVA);
							parts[ID(r)].ctype = PT_HEAC;
						}
					}
					else if (parts[i].ctype == PT_ROCK && rt == PT_LAVA && parts[ID(r)].ctype == PT_GOLD && parts[ID(r)].tmp == 0 &&
						sim->pv[y / CELL][x / CELL] >= 50 && sim->rng.chance(1, 10000)) // Produce GOLD veins/clusters
					{
						parts[i].ctype = PT_GOLD;
						if (rx > 1 || rx < -1) // Trend veins vertical
							parts[i].tmp = 1;
					}
					else if (parts[i].ctype == PT_SALT && rt == PT_GLAS && parts[ID(r)].life < 234 * 120)
					{
						parts[ID(r)].life++;
					}
				}

				if ((surround_space || elements[rt].Explosive) &&
				    elements[rt].Flammable && sim->rng.chance(int(elements[rt].Flammable + (sim->pv[(y+ry)/CELL][(x+rx)/CELL] * 10.0f)), 1000) &&
				    //exceptions, t is the thing causing the spark and rt is what's burning
				    (t != PT_SPRK || (rt != PT_RBDM && rt != PT_LRBD && rt != PT_INSL)) &&
				    (t != PT_PHOT || rt != PT_INSL) &&
				    (rt != PT_SPNG || parts[ID(r)].life == 0))
				{
					sim->part_change_type(ID(r), x+rx, y+ry, PT_FIRE);
					parts[ID(r)].temp = restrict_flt(elements[PT_FIRE].DefaultProperties.temp + (elements[rt].Flammable/2), MIN_TEMP, MAX_TEMP);
					parts[ID(r)].life = sim->rng.between(180, 259);
					parts[ID(r)].tmp = parts[ID(r)].ctype = 0;
					if (elements[rt].Explosive)
						sim->pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
			}
		}
	}
	if (sim->legacy_enable && t!=PT_SPRK) // SPRK has no legacy reactions
		updateLegacy(UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

static int updateLegacy(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int t = parts[i].type;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->bmap[(y+ry)/CELL][(x+rx)/CELL] && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_STREAM)
					continue;
				auto rt = TYP(r);

				auto lpv = (int)sim->pv[(y+ry)/CELL][(x+rx)/CELL];
				if (lpv < 1) lpv = 1;
				if (elements[rt].Meltable &&
				        ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK)
				        && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR))
				        && sim->rng.chance(elements[rt].Meltable*lpv, 1000))
				{
					if (t!=PT_LAVA || parts[i].life>0)
					{
						if (rt==PT_BRMT)
							parts[ID(r)].ctype = PT_BMTL;
						else if (rt==PT_SAND)
							parts[ID(r)].ctype = PT_GLAS;
						else
							parts[ID(r)].ctype = rt;
						sim->part_change_type(ID(r),x+rx,y+ry,PT_LAVA);
						parts[ID(r)].life = sim->rng.between(240, 359);
					}
					else
					{
						parts[i].life = 0;
						parts[i].ctype = PT_NONE;//rt;
						sim->part_change_type(i,x,y,(parts[i].ctype)?parts[i].ctype:PT_STNE);
						return 1;
					}
				}
				if (rt==PT_ICEI || rt==PT_SNOW)
				{
					sim->part_change_type(ID(r), x+rx, y+ry, PT_WATR);
					if (t==PT_FIRE)
					{
						sim->kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						sim->part_change_type(i,x,y,PT_STNE);
					}
				}
				if (rt==PT_WATR || rt==PT_DSTW || rt==PT_SLTW)
				{
					sim->kill_part(ID(r));
					if (t==PT_FIRE)
					{
						sim->kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						parts[i].ctype = PT_NONE;
						sim->part_change_type(i,x,y,(parts[i].ctype)?parts[i].ctype:PT_STNE);
					}
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	RGB color = Renderer::flameTableAt(cpart->life);
	*colr = color.Red;
	*colg = color.Green;
	*colb = color.Blue;

	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = sim->rng.between(120, 169);
}
