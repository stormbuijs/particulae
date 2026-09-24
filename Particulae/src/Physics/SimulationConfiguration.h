#pragma once

#include "../Math/Real.h"



// Centrale plek voor alle parameters,

struct SimulationConfiguration
{
	// Morse-potentiaal

	Real morseWellDepth = 600.0;
	Real morseWidth = 0.25;
	Real morseRestLength = 20.0;


	// Coulomb

	Real coulombConstant = 2000.0;
	Real coulombSoftening = 5.0;


	// Pauli-uitsluiting

	Real pauliRepulsionStrength = 2000.0;
	Real pauliRepulsionRadius = 8.0;


	// Langevin-demping

	Real langevinFriction = 0.5;
	Real langevinNoise = 0.1;


	// Elektronen

	Real electronMass = 0.05;
	Real electronRadius = 3.0;
	Real electronCharge = -1.0;

	Real electronPairDistance = 4.0;
	Real pairMorseWellDepth = 400.0;
	Real pairMorseWidth = 0.8;


	// Nucleus

	Real nucleusRadius = 6.0;


	// Tools

	Real dragSpringStrength = 10.0;


	// Aantal positie-relaxatie-iteraties in PreSolve()
	int preSolveIterationCount = 20;


	// Veiligheidsgrens

	Real minimumDistance = 0.0001;
};