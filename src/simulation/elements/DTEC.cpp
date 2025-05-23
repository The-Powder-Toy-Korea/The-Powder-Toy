#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_DTEC()
{
	Identifier = "DEFAULT_PT_DTEC";
	Name = "DTEC";
	Colour = 0xFD9D18_rgb;
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "탐지기: ctype으로 설정된 입자가 근처에 있을 때 전류를 방출합니다.";

	Properties = TYPE_SOLID;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.tmp2 = 2;

	Update = &update;
	CtypeDraw = &Element::ctypeDrawVInTmp;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int rd = parts[i].tmp2;
	if (rd > 25) parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
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
					auto pavg = sim->parts_avg(i,ID(r),PT_INSL);
					if (pavg != PT_INSL && pavg != PT_RSSS)
					{
						if ((elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[ID(r)].life==0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
						}
					}
				}
			}
		}
	}
	bool setFilt = false;
	int photonWl = 0;
	for (auto rx=-rd; rx<rd+1; rx++)
	{
		for (auto ry=-rd; ry<rd+1; ry++)
		{
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				auto r = pmap[y+ry][x+rx];
				if(!r)
					r = sim->photons[y+ry][x+rx];
				if(!r)
					continue;
				if (TYP(r) == parts[i].ctype && (parts[i].ctype != PT_LIFE || parts[i].tmp == parts[ID(r)].ctype || !parts[i].tmp))
					parts[i].life = 1;
				if (TYP(r) == PT_PHOT || (TYP(r) == PT_BRAY && parts[ID(r)].tmp!=2) || TYP(r) == PT_BIZR || TYP(r) == PT_BIZRG || TYP(r) == PT_BIZRS)
				{
					setFilt = true;
					photonWl = parts[ID(r)].ctype;
				}
			}
		}
	}
	if (setFilt)
	{
		int nx, ny;
		for (auto rx=-1; rx<2; rx++)
		{
			for (auto ry=-1; ry<2; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					nx = x+rx;
					ny = y+ry;
					while (TYP(r)==PT_FILT)
					{
						parts[ID(r)].ctype = photonWl;
						nx += rx;
						ny += ry;
						if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
							break;
						r = pmap[ny][nx];
					}
				}
			}
		}
	}
	return 0;
}
