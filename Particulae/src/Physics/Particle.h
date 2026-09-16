#pragma once

#include "../Math/Vector2.h"



// Eén los deeltje in de simulatie, bijvoorbeeld een nucleus, elektron, of iets anders.
// De radius is puur visueel, aangezien er geen fysieke botsingen kunnen plaatsvinden.

class Particle
{

public:

	Particle(Vector2 position, Real mass, Real radius, Real charge)
		: position(position), mass(mass), radius(radius), charge(charge)
	{

	}


	Vector2 GetPosition() const { return position; }
	Vector2 GetVelocity() const { return velocity; }

	Real GetMass() const { return mass; }
	Real GetRadius() const { return radius; }
	Real GetCharge() const { return charge; }


	void SetPosition(Vector2 newPosition) { position = newPosition; }
	void SetVelocity(Vector2 newVelocity) { velocity = newVelocity; }
	void SetCharge(Real newCharge) { charge = newCharge; }


	// Telt een kracht op bij de accumulatie voor deze stap.
	// Meerdere krachten roepen dit allemaal aan voordat KickVelocity wordt toegepast.
	
	void ApplyForce(Vector2 force) { accumulatedForce += force; }


	// Halve Verlet-stap: past de opgebouwde kracht toe op de snelheid.
	// Wordt twee keer per Step() aangeroepen, met Δt/2.

	void KickVelocity(Real dt)
	{
		Vector2 acceleration = accumulatedForce / mass;
		velocity += acceleration * dt;
	}


	// Werkt de positie bij op basis van de huidige snelheid.

	void DriftPosition(Real dt)
	{
		position += velocity * dt;
	}


	// Zet de opgebouwde kracht terug naar nul; moet na elke evaluatie worden aangeroepen.
	
	void ClearForce() { accumulatedForce = Vector2{}; }



private:

	Vector2 position;
	Vector2 velocity;
	Vector2 accumulatedForce;

	Real mass;
	Real radius;
	Real charge;

};