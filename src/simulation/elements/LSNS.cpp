#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_LSNS()
{
	Identifier = "DEFAULT_PT_LSNS";
	Name = "LSNS";
	Colour = 0x336699_rgb;
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
	Description = "수명값 감지기: 주변에 설정한 온도보다 높은 수명값을 갖는 입자가 있는 경우 전류를 방출합니다.";

	Properties = TYPE_SOLID;

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
		for (int rx = -2; rx <= 2; rx++)
		{
			for (int ry = -2; ry <= 2; ry++)
			{
				if (rx || ry)
				{
					int r = pmap[y + ry][x + rx];
					if (!r)
						r = sim->photons[y + ry][x + rx];
					if (!r)
						continue;
					int rt = TYP(r);
					auto pavg = sim->parts_avg(i, ID(r), PT_INSL);
					if (pavg != PT_INSL && pavg != PT_RSSS)
					{
						if ((elements[rt].Properties&PROP_CONDUCTS) && !(rt == PT_WATR || rt == PT_SLTW || rt == PT_NTCT || rt == PT_PTCT || rt == PT_INWR) && parts[ID(r)].life == 0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r), x + rx, y + ry, PT_SPRK);
						}
					}
				}
			}
		}
	}
	bool doSerialization = false;
	bool doDeserialization = false;
	int life = 0;
	for (int rx = -rd; rx < rd + 1; rx++)
	{
		for (int ry = -rd; ry < rd + 1; ry++)
		{
			if (x + rx >= 0 && y + ry >= 0 && x + rx < XRES && y + ry < YRES && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;

				switch (parts[i].tmp)
				{
				case 1:
					// .life serialization into FILT
					if (TYP(r) != PT_LSNS && TYP(r) != PT_FILT && parts[ID(r)].life >= 0)
					{
						doSerialization = true;
						life = parts[ID(r)].life;
					}
					break;
				case 3:
					// .life deserialization
					if (TYP(r) == PT_FILT)
					{
						doDeserialization = true;
						life = parts[ID(r)].ctype;
					}
					break;
				case 2:
					// Invert mode
					if (TYP(r) != PT_METL && parts[ID(r)].life <= parts[i].temp - 273.15)
						parts[i].life = 1;
					break;
				default:
					// Normal mode
					if (TYP(r) != PT_METL && parts[ID(r)].life > parts[i].temp - 273.15)
						parts[i].life = 1;
					break;
				}
			}
		}
	}

	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					r = sim->photons[y + ry][x + rx];
				if (!r)
					continue;
				int nx = x + rx;
				int ny = y + ry;
				// .life serialization.
				if (doSerialization)
				{
					while (TYP(r) == PT_FILT)
					{
						parts[ID(r)].ctype = 0x10000000 + life;
						nx += rx;
						ny += ry;
						if (nx < 0 || ny < 0 || nx >= XRES || ny >= YRES)
							break;
						r = pmap[ny][nx];
					}
				}
				// .life deserialization.
				else if (doDeserialization)
				{
					if (TYP(r) != PT_FILT)
						parts[ID(r)].life = life - 0x10000000;
				}
			}
		}
	}

	return 0;
}
