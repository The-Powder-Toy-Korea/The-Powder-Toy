#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_DMG()
{
	Identifier = "DEFAULT_PT_DMG";
	Name = "DMG";
	Colour = 0x88FF88_rgb;
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.0f;
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
	Description = "압력 폭탄: 강한 압력을 발생시키고 충돌하는 모든 물체를 부숩니다.";

	Properties = TYPE_PART|PROP_SPARKSETTLE;

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
	int rad = 25;
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_DMG && TYP(r)!=PT_EMBR && TYP(r)!=PT_DMND && TYP(r)!=PT_CLNE && TYP(r)!=PT_PCLN && TYP(r)!=PT_BCLN)
				{
					sim->kill_part(i);
					for (auto nxj=-rad; nxj<=rad; nxj++)
					{
						for (auto nxi=-rad; nxi<=rad; nxi++)
						{
							if (x+nxi>=0 && y+nxj>=0 && x+nxi<XRES && y+nxj<YRES && (nxi || nxj))
							{
								auto dist = int(sqrt(pow(nxi, 2.0f)+pow(nxj, 2.0f)));//;(pow((float)nxi,2))/(pow((float)rad,2))+(pow((float)nxj,2))/(pow((float)rad,2));
								if (!dist || (dist <= rad))
								{
									auto rr = pmap[y+nxj][x+nxi];
									if (rr)
									{
										auto angle = atan2((float)nxj, nxi);
										auto fx = cos(angle) * 7.0f;
										auto fy = sin(angle) * 7.0f;
										parts[ID(rr)].vx += fx;
										parts[ID(rr)].vy += fy;
										sim->vx[(y+nxj)/CELL][(x+nxi)/CELL] += fx;
										sim->vy[(y+nxj)/CELL][(x+nxi)/CELL] += fy;
										sim->pv[(y+nxj)/CELL][(x+nxi)/CELL] += 1.0f;
										auto t = TYP(rr);
										if (t && elements[t].HighPressureTransition>-1 && elements[t].HighPressureTransition<PT_NUM)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, elements[t].HighPressureTransition);
										else if (t == PT_BMTL)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BRMT);
										else if (t == PT_GLAS)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BGLA);
										else if (t == PT_COAL)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BCOL);
										else if (t == PT_QRTZ)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_PQRT);
										else if (t == PT_TUNG)
										{
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BRMT);
											parts[ID(rr)].ctype = PT_TUNG;
										}
										else if (t == PT_WOOD)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_SAWD);
									}
								}
							}
						}
					}
					return 1;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}
