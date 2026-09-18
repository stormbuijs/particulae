#pragma once

#include "../Math/Vector2.h"

#include "Particle.h"
#include "Element.h"
#include "SimulationConfiguration.h"

#include <vector>
#include <cmath>
#include <unordered_set>
#include <utility>
#include <random>



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
	ElementType type{ ElementType::Hydrogen };

	size_t nucleusIndex{ 0 };

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



	// Eén vaste physics-stap met Velocity Verlet met dubbele krachNitevaluatie.

	void Step(Real dt)
	{
		// Eerste krachtevaluatie.

		ApplyAllForces(dt);

		for (Particle& particle : particles)
		{
			particle.KickVelocity(dt / 2.0);
		}

		for (Particle& particle : particles)
		{
			particle.DriftPosition(dt);
		}

		ClearAllForces();


		// Tweede krachtevaluatie op de nieuwe posities na de drift.
		
		ApplyAllForces(dt);

		for (Particle& particle : particles)
		{
			particle.KickVelocity(dt / 2.0);
		}

		ClearAllForces();
	}



	// Eenmalige positie-relaxatie zonder snelheid af te leiden. Dit is nodig, omdat CreateAtom
	// elektronen positioneert op een cirkel rond hun eigen nucleus, maar zodra BondAtoms twee
	// elektronen van verschillende atomen aan elkaar koppelt met een PairingBond, staan die
	// twee elektronen vrijwel zeker niet op de juiste afstand van elkaar.
	// 
	// Zonder deze stap zou de eerste Step() een grote kunstmatige snelheids-kkick geven zodra
	// Morse het grote verschil in één klap probeert te corrigeren.

	void PreSolve()
	{
		for (int iteration = 0; iteration < configuration.preSolveIterationCount; ++iteration)
		{
			for (const Bond& bond : bonds)
			{
				Particle& particleA = particles[bond.particleIndexA];
				Particle& particleB = particles[bond.particleIndexB];


				Vector2 delta = particleB.GetPosition() - particleA.GetPosition();
				Real distance = delta.Length();

				if (distance < configuration.minimumDistance)
				{
					distance = configuration.minimumDistance;
				}

				Vector2 direction = delta / distance;


				Real correction = distance - bond.restLength;


				// Massa-gewogen verdeling: een lichter deeltje verplaatst meer dan een
				// zwaarder deeltje. Zo blijven nuclei grotendeels op hun plek en schikken
				// vooral de elektronen zich, wat ook logsicher is.

				Real totalMass = particleA.GetMass() + particleB.GetMass();

				Real weightA = particleB.GetMass() / totalMass;
				Real weightB = particleA.GetMass() / totalMass;


				particleA.SetPosition(particleA.GetPosition() + direction * (correction * weightA));
				particleB.SetPosition(particleB.GetPosition() - direction * (correction * weightB));
			}
		}
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



	// Comprimeert twee deeltje-indices tot één sleutel, ongeacht de volgorde.
	// Dit wordt gebruikt om PairingBond-paren snel op te kunnen zoeken tijdens de krachtenloop.

	static uint64_t MakePairKey(size_t indexA, size_t indexB)
	{
		if (indexA > indexB)
		{
			std::swap(indexA, indexB);
		}

		return (static_cast<uint64_t>(indexA) << 32) | static_cast<uint64_t>(indexB);
	}



	// Eén keer per Step() worden alle elektronenparen opgebouwd die via een PairingBond aan
	// elkaar gekoppeld zijn. Deze worden dus uitgesloten van Coulomb- en Pauli-krachten met elkaar.

	std::unordered_set<uint64_t> BuildExcludedPairSet() const
	{
		std::unordered_set<uint64_t> excludedPairs;


		for (const Bond& bond : bonds)
		{
			if (bond.kind == BondKind::PairingBond)
			{
				excludedPairs.insert(MakePairKey(bond.particleIndexA, bond.particleIndexB));
			}
		}


		return excludedPairs;
	}



	// Coulomb met Plummer-softening en Pauli-uitsluiting tussen elk paar deeltjes dat niet
	// is uitgesloten. Beide krachten werken langs dezelfde as (de directe lijn tussen de twee
	// deeltjes), dus worden per paar in één keer opgeteld en toegepast.

	void ApplyPairwiseForces()
	{
		std::unordered_set<uint64_t> excludedPairs = BuildExcludedPairSet();


		for (size_t indexA = 0; indexA < particles.size(); ++indexA)
		{
			for (size_t indexB = indexA + 1; indexB < particles.size(); ++indexB)
			{
				if (excludedPairs.contains(MakePairKey(indexA, indexB)))
				{
					continue;
				}


				Vector2 delta = particles[indexB].GetPosition() - particles[indexA].GetPosition();
				Real distance = delta.Length();

				if (distance < configuration.minimumDistance)
				{
					distance = configuration.minimumDistance;
				}

				Vector2 direction = delta / distance;


				// Coulomb: F = k_e * q1 * q2 / (r^2 + softening^2).
				// Bij een gelijke lading positief, dus afstotend,
				// bij een ongelijke lading negatief, dus aantrekkend.

				Real softenedDistanceSquared = distance * distance + configuration.coulombSoftening * configuration.coulombSoftening;
				Real coulombMagnitude = configuration.coulombConstant * particles[indexA].GetCharge() * particles[indexB].GetCharge() / softenedDistanceSquared;


				// Pauli: F = strength * (radius / r)^12.
				// Altijd afstotend, ongeacht de lading.

				Real pauliMagnitude = configuration.pauliRepulsionStrength * std::pow(configuration.pauliRepulsionRadius / distance, 12.0);


				Vector2 totalForce = direction * (coulombMagnitude + pauliMagnitude);


				// De derde wet van Newton: tegengestelde kracht op beide deeltjes.

				particles[indexA].ApplyForce(-totalForce);
				particles[indexB].ApplyForce(totalForce);
			}
		}
	}



	// Morse-potentiaal per binding: F(r) = 2 * D_e * a * (exp(-a(r - r0)) - exp(-2a(r - r0))).
	// Geld voor zowel AtomicBond en PairingBond, met eigen parameters per type.
	// Deze bindingen zijn breekbaar: bij grote uirekking naderen beide exp-termen naar 0.

	void ApplyBondForces()
	{
		for (const Bond& bond : bonds)
		{
			bool isAtomicBond = (bond.kind == BondKind::AtomicBond);
			
			Real wellDepth = isAtomicBond ? configuration.morseWellDepth : configuration.pairMorseWellDepth;
			Real width = isAtomicBond ? configuration.morseWidth : configuration.pairMorseWidth;


			Vector2 delta = particles[bond.particleIndexB].GetPosition() - particles[bond.particleIndexA].GetPosition();
			Real distance = delta.Length();

			if (distance < configuration.minimumDistance)
			{
				distance = configuration.minimumDistance;
			}

			Vector2 direction = delta / distance;


			Real exponent = std::exp(-width * (distance - bond.restLength));
			Real morseForceMagnitude = 2.0 * wellDepth * width * (exponent - exponent * exponent);


			// Postieve morseForceMagnitude is een binding die uitrekt (r > r0):
			// de kracht terkt de twee deeltjes naar elkaar toe.

			particles[bond.particleIndexA].ApplyForce(direction * morseForceMagnitude);
			particles[bond.particleIndexB].ApplyForce(direction * (-morseForceMagnitude));
		}
	}



	// Langevin-demping: wrijving (-γv) + thermische ruis.
	// De ruis wordt geschaald met 1 / sqrt(Δt) zodat het effectieve temperatuurgedrag
	// onafhankelijk blijft van de gekozen update-rate.

	void ApplyLangevinForces(Real dt)
	{
		std::normal_distribution<Real> distribution(0.0, 1.0);

		for (Particle& particle : particles)
		{
			Vector2 frictionForce = particle.GetVelocity() * (-configuration.langevinFriction);

			Vector2 noiseForce{
				configuration.langevinNoise * distribution(randomEngine) / std::sqrt(dt),
				configuration.langevinNoise * distribution(randomEngine) / std::sqrt(dt)
			};


			particle.ApplyForce(frictionForce + noiseForce);
		}
	}



	// Berekent alle krachten voor deze evaluaite.
	// Wordt twee keer per Step() aangeroepen: op de oude positie en op de nieuwe positie.

	void ApplyAllForces(Real dt)
	{
		ApplyPairwiseForces();
		ApplyBondForces();
		ApplyLangevinForces(dt);
	}


	void ClearAllForces()
	{
		for (Particle& particle : particles)
		{
			particle.ClearForce();
		}
	}



	SimulationConfiguration configuration;

	std::vector<Particle> particles;
	std::vector<Bond> bonds;


	std::mt19937 randomEngine{ std::random_device{}() };

};