#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BRAY()
{
	Identifier = "DEFAULT_PT_BRAY";
	Name = "BRAY";
	Colour = 0xFFFFFF_rgb;
	MenuVisible = 0;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
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

	HeatConduct = 251;
	Description = "B형 광선 방출체: 광선끼리 충돌하면 점을 생성합니다.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 30;

	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int x, trans = 255;
	if(cpart->tmp==0)
	{
		trans = cpart->life * 7;
		if (trans>255) trans = 255;
		if (cpart->ctype&0x3FFFFFFF) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (cpart->ctype >> (x+18)) & 1;
				*colb += (cpart->ctype >>  x)	 & 1;
			}
			for (x=0; x<12; x++)
				*colg += (cpart->ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(cpart->tmp==1)
	{
		trans = cpart->life/4;
		if (trans>255) trans = 255;
		if (cpart->ctype&0x3FFFFFFF) {
			*colg = 0;
			*colb = 0;
			*colr = 0;
			for (x=0; x<12; x++) {
				*colr += (cpart->ctype >> (x+18)) & 1;
				*colb += (cpart->ctype >>  x)	 & 1;
			}
			for (x=0; x<12; x++)
				*colg += (cpart->ctype >> (x+9))  & 1;
			x = 624/(*colr+*colg+*colb+1);
			*colr *= x;
			*colg *= x;
			*colb *= x;
		}
	}
	else if(cpart->tmp==2)
	{
		trans = cpart->life*100;
		if (trans>255) trans = 255;
		*colr = 255;
		*colg = 150;
		*colb = 50;
	}
	*cola = trans;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_BLEND | PMODE_GLOW;
	return 0;
}
