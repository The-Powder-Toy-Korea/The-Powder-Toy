#include "simulation/ElementCommon.h"
#include "simulation/orbitalparts.h"
#include "PIPE.h"
#include "SOAP.h"
#include "PRTI.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PRTI()
{
	Identifier = "DEFAULT_PT_PRTI";
	Name = "PRTI";
	Colour = 0xEB5917_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = -0.005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 0;
	Description = "포털 입구: 입자가 여기로 들어갑니다. 같은 온도 채널의 포털 출구로 입자를 즉시 전송합니다(WIFI와 동일).";

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

/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	int fe = 0;

	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp >= CHANNELS)
		parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp < 0)
		parts[i].tmp = 0;

	for (int count = 0; count < 8; count++)
	{
		int rx = portal_rx[count];
		int ry = portal_ry[count];
		if (rx || ry)
		{
			int r = pmap[y+ry][x+rx];
			if (!r || TYP(r) == PT_STOR)
				fe = 1;
			if (!r || (!(elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)) && TYP(r)!=PT_SPRK && TYP(r)!=PT_STOR))
			{
				r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
			}

			if (TYP(r)==PT_STKM || TYP(r)==PT_STKM2 || TYP(r)==PT_FIGH)
				continue;// Handling these is a bit more complicated, and is done in STKM_interact()

			if (TYP(r) == PT_SOAP)
				Element_SOAP_detach(sim, ID(r));

			for (int nnx=0; nnx<80; nnx++)
				if (!sim->portalp[parts[i].tmp][count][nnx].type)
				{
					if (TYP(r) == PT_STOR)
					{
						if (sd.IsElement(parts[ID(r)].tmp) && (elements[parts[ID(r)].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
						{
							// STOR uses same format as PIPE, so we can use this function to do the transfer
							Element_PIPE_transfer_pipe_to_part(sim, parts+(ID(r)), &sim->portalp[parts[i].tmp][count][nnx], true);
							break;
						}
					}
					else
					{
						sim->portalp[parts[i].tmp][count][nnx] = parts[ID(r)];
						if (TYP(r) == PT_SPRK)
							sim->part_change_type(ID(r),x+rx,y+ry,parts[ID(r)].ctype);
						else
							sim->kill_part(ID(r));
						fe = 1;
						break;
					}
				}
		}
	}


	if (fe) {
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!sim->parts[i].life) parts[i].life = sim->rng.gen();
		if (!sim->parts[i].ctype) parts[i].ctype = sim->rng.gen();
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (int r = 0; r < 4; r++) {
			if (orbd[r]>1) {
				orbd[r] -= 12;
				if (orbd[r]<1) {
					orbd[r] = sim->rng.between(128, 255);
					orbl[r] = sim->rng.between(0, 254);
				} else {
					orbl[r] += 2;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = sim->rng.between(128, 255);
				orbl[r] = sim->rng.between(0, 254);
			}
		}
		orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	} else {
		parts[i].life = 0;
		parts[i].ctype = 0;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 8;
	*firer = 255;
	*fireg = 0;
	*fireb = 0;
	*pixel_mode |= EFFECT_DBGLINES;
	*pixel_mode |= EFFECT_GRAVIN;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
