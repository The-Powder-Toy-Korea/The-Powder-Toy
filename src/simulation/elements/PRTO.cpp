#include "simulation/ElementCommon.h"
#include "simulation/orbitalparts.h"
#include "PRTI.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PRTO()
{
	Identifier = "DEFAULT_PT_PRTO";
	Name = "PRTO";
	Colour = 0x0020EB_rgb;
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
	HotAir = 0.005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 0;
	Description = "포털 출구: 입자가 여기서 나옵니다. 같은 온도 채널의 포털 입구에서 입자가 즉시 전송됩니다(WIFI와 동일).";

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
	int fe = 0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (auto count=0; count<8; count++)
	{
		auto rx = portal_rx[count];
		auto ry = portal_ry[count];
		if (rx || ry)
		{
			auto r = pmap[y+ry][x+rx];
			if (!r)
			{
				fe = 1;
				for (auto nnx =0 ; nnx<80; nnx++)
				{
					int randomness = (count + sim->rng.between(-1, 1) + 4) % 8;//add -1,0,or 1 to count
					if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_SPRK)// TODO: make it look better, spark creation
					{
						sim->create_part(-1,x+1,y,PT_SPRK);
						sim->create_part(-1,x+1,y+1,PT_SPRK);
						sim->create_part(-1,x+1,y-1,PT_SPRK);
						sim->create_part(-1,x,y-1,PT_SPRK);
						sim->create_part(-1,x,y+1,PT_SPRK);
						sim->create_part(-1,x-1,y+1,PT_SPRK);
						sim->create_part(-1,x-1,y,PT_SPRK);
						sim->create_part(-1,x-1,y-1,PT_SPRK);
						memset(&sim->portalp[parts[i].tmp][randomness][nnx], 0, sizeof(Particle));
						break;
					}
					else if (sim->portalp[parts[i].tmp][randomness][nnx].type)
					{
						if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
							sim->player.spwn = 0;
						if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
							sim->player2.spwn = 0;
						if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
						{
							sim->fighcount--;
							sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 0;
						}
						auto np = sim->create_part(-1, x+rx, y+ry, sim->portalp[parts[i].tmp][randomness][nnx].type);
						if (np<0)
						{
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
								sim->player.spwn = 1;
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
								sim->player2.spwn = 1;
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
							{
								sim->fighcount++;
								sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
							}
							continue;
						}
						if (parts[np].type==PT_FIGH)
						{
							// Release the fighters[] element allocated by create_part, the one reserved when the fighter went into the portal will be used
							sim->fighters[(unsigned char)parts[np].tmp].spwn = 0;
							sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
						}
						if (sim->portalp[parts[i].tmp][randomness][nnx].vx == 0.0f && sim->portalp[parts[i].tmp][randomness][nnx].vy == 0.0f)
						{
							// particles that have passed from PIPE into PRTI have lost their velocity, so use the velocity of the newly created particle if the particle in the portal has no velocity
							float tmp_vx = parts[np].vx;
							float tmp_vy = parts[np].vy;
							parts[np] = sim->portalp[parts[i].tmp][randomness][nnx];
							parts[np].vx = tmp_vx;
							parts[np].vy = tmp_vy;
						}
						else
							parts[np] = sim->portalp[parts[i].tmp][randomness][nnx];
						parts[np].x = float(x+rx);
						parts[np].y = float(y+ry);
						memset(&sim->portalp[parts[i].tmp][randomness][nnx], 0, sizeof(Particle));
						break;
					}
				}
			}
		}
	}
	if (fe)
	{
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!sim->parts[i].life) parts[i].life = sim->rng.gen();
		if (!sim->parts[i].ctype) parts[i].ctype = sim->rng.gen();
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (auto r = 0; r < 4; r++)
		{
			if (orbd[r]<254)
			{
				orbd[r] += 16;
				if (orbd[r]>254) {
					orbd[r] = 0;
					orbl[r] = sim->rng.between(0, 254);
				}
				else
				{
					orbl[r] += 1;
					orbl[r] = orbl[r]%255;
				}
			}
			else
			{
				orbd[r] = 0;
				orbl[r] = sim->rng.between(0, 254);
			}
		}
		orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	}
	else
	{
		parts[i].life = 0;
		parts[i].ctype = 0;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 8;
	*firer = 0;
	*fireg = 0;
	*fireb = 255;
	*pixel_mode |= EFFECT_DBGLINES;
	*pixel_mode |= EFFECT_GRAVOUT;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
