#pragma once

#include "../Math/Vector2.h"



// Bepaalt welk deel van de simulatiewereld zichtbaar is.

class Camera
{

public:

	Vector2 GetPosition() const { return position; }
	Real GetZoom() const { return zoom; }

	void SetPosition(Vector2 newPosition) { position = newPosition; }
	void SetZoom(Real newZoom) { zoom = newZoom; }
	void Pan(Vector2 delta) { position += delta; }

private:

	Vector2 position{ 0.0, 0.0 };
	Real zoom = 1.0;

};