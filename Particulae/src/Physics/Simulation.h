#pragma once

#include "../Math/Vector2.h"

#include "Particle.h"
#include "Element.h"
#include "SimulationConfiguration.h"

#include <vector>
#include <cmath>



enum class BondKind
{
	AtomicBond, PairingBond
};



struct Bond
{
	size_t particleIndexA;
	size_t particleIndexB;

	Real restLength;

	BondKind kind;
};


struct Atom
{
	ElementType type;

	size_t nucleusIndex;

	std::vector<size_t> electronIndicies;
};



class Simulation
{

public:

	size_t AddParticle(const Particle& particle)
	{
		particles.push_back(particle);
		return particles.size() - 1;
	}


	void AddBond(size_t indexA, size_t indexB, Real restLength, BondKind kind)
	{
		bonds.push_back({ indexA, indexB, restLength, kind });
	}



	// Maakt de nucleus en alle losse valantie-elektronen van een atoom aan.

	// De eerste bondCapacity elektronen blijven ongekoppend voor het binden van atomen,
	// de resterende lonePairCount * 2 elektronen worden meteen als vrije paren aan elkaar
	// gekoppeld met een ParingBond. Zonder deze koppeling zouden ze elkaar via Coulomb juist
	// uit elkaar drijven in plaats van als paar ruimte opnemen.

	// Elektronen worden bij het begin al op morseRestLength van de nucleus gezet,
	// zodat er geen kunstmatige snelheidsonregelmatigheid ontstaat bij de eerste evaluatie.

	Atom CreateAtom(ElementType type, Vector2 position)
	{
		ElementData elementData = GetElementData(type);
		Real valenceElectronCount = GetValanceElectronCount(elementData);


		// Effectieve kernlading is het aantal valentie-elektronen,
		// zodat het atoom als los geheel neutraal is.

		Particle nucleus(position, elementData.mass, configuration.nucleusRadius, valenceElectronCount);
		size_t nucleusIndex = AddParticle(nucleus);


		Atom atom;
		atom.type = type;
		atom.nucleusIndex = nucleusIndex;


		int electronCount = static_cast<int>(valenceElectronCount);
		
		for (int index = 0; index < electronCount; ++index)
		{
			Real angle = (2.0 * pi * static_cast<Real>(index)) / static_cast<Real>(electronCount);
			Vector2 offset{ std::cos(angle) * configuration.morseRestLength, std::sin(angle) * configuration.morseRestLength };


			Particle electron(position + offset, configuration.electronMass, configuration.electronRadius, configuration.electronCharge);
			size_t electronIndex = AddParticle(electron);

			atom.electronIndicies.push_back(electronIndex);


			AddBond(nucleusIndex, electronIndex, configuration.morseRestLength, BondKind::AtomicBond);
		}


		// De bondCapacity aantal ongekoppelde elektronen worden in opeenvolgende paren gekoppeld.

		int bondingElectronCount = elementData.bondCapacity;

		for (int index = bondingElectronCount; index + 1 < electronCount; index += 2)
		{
			AddBond(
				atom.electronIndicies[index],
				atom.electronIndicies[index + 1],
				configuration.electronPairDistance,
				BondKind::PairingBond
			);
		}


		return atom;
	}



	// Verbind één ongekoppeld elektron van elk atoom met een ParingBond,
	// hierdoor ontstaat de gedeelde binding tussen de twee atomen.

	void BondAtoms(Atom& atomA, Atom& atomB)
	{
		size_t electronA = FindUnpairedElectron(atomA);
		size_t electronB = FindUnpairedElectron(atomB);

		AddBond(electronA, electronB, configuration.electronPairDistance, BondKind::PairingBond);
	}



	// Eén vaste physics-stap.

	void Step(Real dt)
	{

	}



	const std::vector<Particle>& GetParticles() const { return particles; }


private:

	static constexpr Real pi = 3.14159265;



	bool isElectronPaired(size_t electronIndex) const
	{
		for (const Bond& bond : bonds)
		{
			bool bondContainsElectron = bond.particleIndexA == electronIndex || bond.particleIndexB == electronIndex;

			if (bond.kind == BondKind::PairingBond && bondContainsElectron)
			{
				return true;
			}
		}

		return false;
	}


	size_t FindUnpairedElectron(const Atom& atom) const
	{
		for (size_t electronIndex : atom.electronIndicies)
		{
			if (!isElectronPaired(electronIndex))
			{
				return electronIndex;
			}
		}


		// Geen ongekoppeld elektron beschikbaar.

		return atom.electronIndicies.front();
	}



	SimulationConfiguration configuration;

	std::vector<Particle> particles;
	std::vector<Bond> bonds;

};