#pragma once

#include "../Physics/Simulation.h"
#include "../Rendering/Camera.h"

#include <SFML/Graphics.hpp>
#include <optional>



class Engine
{

public:

	Engine() : window(sf::VideoMode({ 1280u, 720u }), "Particulae")
	{

	}


	void Start()
	{

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
			}


			Real frameTime = clock.restart().asSeconds();
			physicsAccumulator += frameTime;


			while (physicsAccumulator >= physicsTimestep)
			{
				simulation.Step(physicsTimestep);
				physicsAccumulator -= physicsTimestep;
			}


			window.clear(sf::Color::Black);
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