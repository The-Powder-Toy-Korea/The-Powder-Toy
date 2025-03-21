#include "simulation/ElementCommon.h"
#include <algorithm>

static int update(UPDATE_FUNC_ARGS);

void Element::Element_FWRK()
{
	Identifier = "DEFAULT_PT_FWRK";
	Name = "FWRK";
	Colour = 0x666666_rgb;
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 97;

	HeatConduct = 100;
	Description = "이전 버전의 폭죽: 열과 중성자에 반응하는 기존의 폭죽입니다.";

	Properties = TYPE_PART|PROP_LIFE_DEC;

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
	if (parts[i].life == 0 && ((surround_space && parts[i].temp>400 && sim->rng.chance(int(9+parts[i].temp/40), 100000)) || parts[i].ctype == PT_DUST))
	{
		float gx, gy, multiplier, gmax;
		int randTmp;
		sim->GetGravityField(x, y, elements[PT_FWRK].Gravity, 1.0f, gx, gy);
		if (gx*gx+gy*gy < 0.001f)
		{
			float angle = sim->rng.between(0, 6283) * 0.001f;//(in radians, between 0 and 2*pi)
			gx += sinf(angle)*elements[PT_FWRK].Gravity*0.5f;
			gy += cosf(angle)*elements[PT_FWRK].Gravity*0.5f;
		}
		gmax = std::max(fabsf(gx), fabsf(gy));
		if (sim->eval_move(PT_FWRK, (int)(x-(gx/gmax)+0.5f), (int)(y-(gy/gmax)+0.5f), nullptr))
		{
			multiplier = 15.0f/sqrtf(gx*gx+gy*gy);

			//Some variation in speed parallel to gravity direction
			randTmp = sim->rng.between(-100, 100);
			gx += gx*randTmp*0.002f;
			gy += gy*randTmp*0.002f;
			//and a bit more variation in speed perpendicular to gravity direction
			randTmp = sim->rng.between(-100, 100);
			gx += -gy*randTmp*0.005f;
			gy += gx*randTmp*0.005f;

			parts[i].life = sim->rng.between(18, 27);
			parts[i].ctype=0;
			parts[i].vx -= gx*multiplier;
			parts[i].vy -= gy*multiplier;
			return 0;
		}
	}
	if (parts[i].life<3&&parts[i].life>0)
	{
		int r = sim->rng.between(11, 255);
		int g = sim->rng.between(11, 255);
		int b = sim->rng.between(11, 255);
		int n;
		float angle, magnitude;
		unsigned col = (r<<16) | (g<<8) | b;
		for (n=0; n<40; n++)
		{
			int np = sim->create_part(-3, x, y, PT_EMBR);
			if (np>-1)
			{
				magnitude = sim->rng.between(40, 99) * 0.05f;
				angle = sim->rng.between(0, 6283) * 0.001f;//(in radians, between 0 and 2*pi)
				parts[np].vx = parts[i].vx*0.5f + cosf(angle)*magnitude;
				parts[np].vy = parts[i].vy*0.5f + sinf(angle)*magnitude;
				parts[np].ctype = col;
				parts[np].tmp = 1;
				parts[np].life = sim->rng.between(70, 109);
				parts[np].temp = float(sim->rng.between(5750, 6249));
				parts[np].dcolour = parts[i].dcolour;
			}
		}
		sim->pv[y/CELL][x/CELL] += 8.0f;
		sim->kill_part(i);
		return 1;
	}
	if (parts[i].life>=45)
		parts[i].life=0;
	return 0;
}
