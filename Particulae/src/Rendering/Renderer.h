#pragma once

#include "../Physics/Particle.h"

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>



// Tekent alle deeltjes als cirkels via één vertex array en één gedeelde cirkeltextuur.

class Renderer
{

public:

	Renderer()
	{
		GenerateCircleTexture();
	}



	// Herbouwt de vertex array op basis van de huidige deeltjesposities.

	void Update(const std::vector<Particle>& particles)
	{
		vertices.clear();
		vertices.setPrimitiveType(sf::PrimitiveType::Triangles);


		for (const Particle& particle : particles)
		{
			AppendParticleQuad(particle);
		}
	}



	void Draw(sf::RenderTarget& target) const
	{
		sf::RenderStates states;
		states.texture = &circleTexture;

		target.draw(vertices, states);
	}

private:

	// Bouwt één herbruikbare cirkeltextuur op met een zachte rand.

	void GenerateCircleTexture()
	{
		constexpr unsigned int textureSize = 64;
		sf::Image image({ textureSize, textureSize }, sf::Color::Transparent);

		float center = static_cast<float>(textureSize) / 2.0f;
		float radius = center;


		for (unsigned int y = 0; y < textureSize; ++y)
		{
			for (unsigned int x = 0; x < textureSize; ++x)
			{

				float distanceX = static_cast<float>(x) + 0.5f - center;
				float distanceY = static_cast<float>(y) + 0.5f - center;


				float distance = std::sqrt(distanceX * distanceX + distanceY * distanceY);
				
				if (distance <= radius)
				{
					image.setPixel({ x, y }, sf::Color::White);
				}
			}
		}


		if (!circleTexture.loadFromImage(image))
		{
			// Zou hier niet moeten optreden, dus een lege instructie,
			// omdat de computer anders gaat waarschuwen.
		}

		circleTexture.setSmooth(true);
	}



	// Kleurt een deeltje op basis van zijn lading; puur educatief om de lading beter te herkennen.

	sf::Color GetColorForCharge(Real charge) const
	{
		// Blauw voor negatieve ladingen.

		if (charge < 0.0)
		{
			return sf::Color(80, 140, 255);
		}


		// Rood voor positieve ladingen.
		
		if (charge > 0.0)
		{
			return sf::Color(255, 90, 90);
		}


		// Grijs voor neutrale ladingen.

		return sf::Color(180, 180, 180);
	}



	// Voegt twee driehoeken (één quad) toe aan de vertex array voor één particle.
	// Deze is gecentreerd op zijn positie, zijde 2 * radius, met de volledige cirkeltextuur.

	void AppendParticleQuad(const Particle& particle)
	{
		float positionX = static_cast<float>(particle.GetPosition().x);
		float positionY = static_cast<float>(particle.GetPosition().y);

		float radius = static_cast<float>(particle.GetRadius());

		sf::Color color = GetColorForCharge(particle.GetCharge());


		sf::Vector2f topLeft{ positionX - radius, positionY - radius };
		sf::Vector2f topRight{ positionX + radius, positionY - radius };
		sf::Vector2f bottomLeft{ positionX - radius, positionY + radius };
		sf::Vector2f bottomRight{ positionX + radius, positionY + radius };


		sf::Vector2f textureSize{
			static_cast<float>(circleTexture.getSize().x),
			static_cast<float>(circleTexture.getSize().y)
		};


		// De eerste driehoek van de quad: linksboven, rechtsboven en rechtsonder.

		vertices.append({ topLeft, color, { 0.0f, 0.0f } });
		vertices.append({ topRight, color, { textureSize.x, 0.0f } });
		vertices.append({ bottomRight, color, { textureSize.x, textureSize.y } });


		// De tweede driehoek van de quad: linksboven, rechtsonder en linksonder.

		vertices.append({ topLeft, color, { 0.0f, 0.0f } });
		vertices.append({ bottomRight, color, { textureSize.x, textureSize.y } });
		vertices.append({ bottomLeft, color, { 0.0f, textureSize.y } });
	}



	sf::Texture circleTexture;
	sf::VertexArray vertices;
};