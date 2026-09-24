#pragma once

#include "../Rendering/Renderer.h"
#include "../Rendering/Camera.h"

#include "../Physics/Simulation.h"

#include <SFML/Graphics.hpp>
#include <optional>



class Engine
{

public:

	Engine() : window(sf::VideoMode({ 1280u, 720u }), "Particulae")
	{
		window.setView(view);
	}


	void Start()
	{
		Atom oxygenAtom = simulation.CreateAtom(ElementType::Oxygen, Vector2{ 0.0, 0.0 });
		Atom hydrogenAAtom = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ -70, -20.0 });
		Atom hydrogenBAtom = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ 60.0, -25.0 });

		simulation.BondAtoms(oxygenAtom, hydrogenAAtom);
		simulation.BondAtoms(oxygenAtom, hydrogenBAtom);


		Atom carbonAtom = simulation.CreateAtom(ElementType::Carbon, Vector2{ 250.0, 0.0 });
		Atom methaneHydrogenA = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ 180.0, -55.0 });
		Atom methaneHydrogenB = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ 305.0, -50.0 });
		Atom methaneHydrogenC = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ 210.0, 60.0 });
		Atom methaneHydrogenD = simulation.CreateAtom(ElementType::Hydrogen, Vector2{ 295.0, 55.0 });

		simulation.BondAtoms(carbonAtom, methaneHydrogenA);
		simulation.BondAtoms(carbonAtom, methaneHydrogenB);
		simulation.BondAtoms(carbonAtom, methaneHydrogenC);
		simulation.BondAtoms(carbonAtom, methaneHydrogenD);

		
		simulation.PreSolve();
	}


	void Run()
	{
		while (window.isOpen())
		{
			while (const std::optional event = window.pollEvent())
			{
				if (event->is<sf::Event::Closed>())
				{
					window.close();
				}
				else if (const auto* resized = event->getIf<sf::Event::Resized>())
				{
					AdjustViewToWindow(resized->size.x, resized->size.y);
				}
			}


			Real frameTime = clock.restart().asSeconds();
			physicsAccumulator += frameTime;


			while (physicsAccumulator >= physicsTimestep)
			{
				simulation.Step(physicsTimestep);
				physicsAccumulator -= physicsTimestep;
			}


			UpdateViewFromCamera();

			renderer.Update(simulation.GetParticles());

			window.clear(sf::Color::Black);
			renderer.Draw(window);
			window.display();
		}
	}

private:

	// Combineert de camerapositie met de huidige view-viewport tot de uiteindelijk sf::View.

	void UpdateViewFromCamera()
	{

		Vector2 position = camera.GetPosition();
		Real zoom = camera.GetZoom();

		view.setCenter({ static_cast<float>(position.x), static_cast<float>(position.y) });
		view.setSize({ baseViewSize.x / static_cast<float>(zoom), baseViewSize.y / static_cast<float>(zoom) });

		window.setView(view);
	}


	// Past alleen de viewport aan zodat baseViewSize altijd op de juiste verhouding wordt getoond.

	void AdjustViewToWindow(unsigned int width, unsigned int height)
	{
		Real windowRatio = static_cast<Real>(width) / static_cast<Real>(height);
		Real viewRatio = static_cast<Real>(baseViewSize.x) / static_cast<Real>(baseViewSize.y);


		Real sizeX = 1.0;
		Real sizeY = 1.0;

		Real positionX = 0.0;
		Real positionY = 0.0;


		if (windowRatio < viewRatio)
		{
			sizeY = windowRatio / viewRatio;
			positionY = (1.0 - sizeY) / 2.0;
		}
		else
		{
			sizeX = viewRatio / windowRatio;
			positionX = (1.0 - sizeX) / 2.0;
		}


		view.setViewport(
			sf::FloatRect(
				{ static_cast<float>(positionX), static_cast<float>(positionY) },
				{ static_cast<float>(sizeX), static_cast<float>(sizeY) }
			)
		);


		window.setView(view);
	}



	Simulation simulation;
	Renderer renderer;

	sf::Clock clock;
	sf::RenderWindow window;


	Camera camera;

	sf::Vector2f baseViewSize{ 1280.f, 720.f };
	sf::View view;


	// De simulatie staat vast op 1000 Hz, omdat onderdelen
	// zoals de integratie en Langevin-ruis een vaste Δt vereisen.

	Real physicsTimestep = 1.0 / 1000.0;
	Real physicsAccumulator = 0.0;

};