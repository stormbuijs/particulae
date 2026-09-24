#pragma once

#include "Tool.h"

#include "../Physics/Simulation.h"

#include <optional>



class DragTool : public Tool
{

public:

	explicit DragTool(Simulation& simulation) : simulation(simulation) {}



	void OnMousePressed(Vector2 worldPosition) override
	{
		draggedParticleIndex = FindNearestParticle(worldPosition);

		if (draggedParticleIndex) {
			simulation.SetDragTarget(*draggedParticleIndex, worldPosition);
		}
	}


	void OnMouseMoved(Vector2 worldPosition) override
	{
		if (draggedParticleIndex)
		{
			simulation.SetDragTarget(*draggedParticleIndex, worldPosition);
		}
	}


	void OnMouseReleased() override
	{
		if (draggedParticleIndex)
		{
			simulation.ClearDragTarget();
			draggedParticleIndex.reset();
		}
	}


private:

	std::optional<size_t> FindNearestParticle(Vector2 worldPosition) const
	{
		const std::vector<Particle>& particles = simulation.GetParticles();

		std::optional<size_t> nearestIndex;
		Real nearestDistance = pickRadius;

		for (size_t index = 0; index < particles.size(); ++index)
		{
			Real distance = (particles[index].GetPosition() - worldPosition).Length();

			if (distance < nearestDistance)
			{
				nearestDistance = distance;
				nearestIndex = index;
			}
		}

		return nearestIndex;
	}



	Simulation& simulation;
	std::optional<size_t> draggedParticleIndex;

	Real pickRadius = 15.0;

};