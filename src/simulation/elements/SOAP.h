#pragma once
#include "simulation/ElementDefs.h"

void Element_SOAP_detach(Simulation * sim, int i);
void Element_SOAP_neighourLoop(const RenderableSimulation *sim, float &dx, float &dy);
