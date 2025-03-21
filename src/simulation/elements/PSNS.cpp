#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_PSNS()
{
	Identifier = "DEFAULT_PT_PSNS";
	Name = "PSNS";
	Colour = 0xDB2020_rgb;
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

	DefaultProperties.temp = 4.0f + 273.15f;
	HeatConduct = 0;
	Description = "압력 감지기: 압력이 설정된 온도보다 높을 때 전류를 방출합니다.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if ((parts[i].tmp == 0 && sim->pv[y/CELL][x/CELL] > parts[i].temp-273.15f) || (parts[i].tmp == 2 && sim->pv[y/CELL][x/CELL] < parts[i].temp-273.15f))
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
					auto pavg = sim->parts_avg(i,ID(r),PT_INSL);
					if (pavg != PT_INSL && pavg != PT_RSSS)
					{
						auto rt = TYP(r);
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
	if (parts[i].tmp == 1)
	{
		bool setFilt = true;
		float photonWl = sim->pv[y / CELL][x / CELL];
		if (setFilt)
		{
			for (auto rx = -1; rx <= 1; rx++)
			{
				for (auto ry = -1; ry <= 1; ry++)
				{
					if (rx || ry)
					{
						auto r = pmap[y + ry][x + rx];
						if (!r)
							continue;
						auto nx = x + rx;
						auto ny = y + ry;
						while (TYP(r) == PT_FILT)
						{
							parts[ID(r)].ctype = 0x10000000 + int(round(photonWl) - MIN_PRESSURE);
							nx += rx;
							ny += ry;
							if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
								break;
							r = pmap[ny][nx];
						}
					}
				}
			}
		}
	}
	return 0;
}
