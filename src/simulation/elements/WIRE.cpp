#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_WIRE()
{
	Identifier = "DEFAULT_PT_WIRE";
	Name = "WWLD";
	Colour = 0xFFCC00_rgb;
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 250;
	Description = "WireWorld 전선: 생명 게임과 유사한 일련의 규칙에 따라 전류를 전달합니다.";

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
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	int count=0;
	/*
	  0:  wire
	  1:  spark head
	  2:  spark tail

	  tmp is previous state, ctype is current state
	*/
	//parts[i].tmp=parts[i].ctype;
	parts[i].ctype=0;
	if (parts[i].tmp==1)
	{
		parts[i].ctype=2;
	}
	else if (parts[i].tmp==2)
	{
		parts[i].ctype=0;
	}
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)==PT_SPRK && parts[ID(r)].life==3 && parts[ID(r)].ctype==PT_PSCN)
				{
					parts[i].ctype=1;
					return 0;
				}
				else if (TYP(r)==PT_NSCN && parts[i].tmp==1)
					sim->create_part(-1, x+rx, y+ry, PT_SPRK);
				else if (TYP(r)==PT_WIRE && parts[ID(r)].tmp==1 && !parts[i].tmp)
					count++;
			}
		}
	}
	if (count==1 || count==2)
		parts[i].ctype=1;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->ctype==0)
	{
		*colr = 255;
		*colg = 204;
		*colb = 0;
		return 0;
	}
	if (cpart->ctype==1)
	{
		*colr = 50;
		*colg = 100;
		*colb = 255;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
	if (cpart->ctype==2)
	{
		*colr = 255;
		*colg = 100;
		*colb = 50;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
	return 0;
}
