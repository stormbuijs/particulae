#pragma once

#include "../Math/Vector2.h"



// Interface voor elke editor-tool (slepen, plaatsen, camera pannen, enzovoorts).
// De engine roept deze hooks aan op basis van de muisinput, zonder te weten welke tool
// actief is. Wisselen van tool kan door de actieve Tool-pointer in de Engine te vervangen.

class Tool
{

public:

	virtual ~Tool() = default;

	virtual void OnMousePressed(Vector2 worldPosition) {}
	virtual void OnMouseMoved(Vector2 worldPosition) {}
	virtual void OnMouseReleased() {}

};