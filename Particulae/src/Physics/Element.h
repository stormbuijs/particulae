#pragma once

#include "../Math/Real.h"



enum class ElementType
{
	Hydrogen, Carbon, Oxygen
};



struct ElementData
{
	Real mass;

	int lonePairCount;
	int bondCapacity;
};



inline Real GetValanceElectronCount(const ElementData& data)
{
	return static_cast<Real>(data.bondCapacity + 2 * data.lonePairCount);
}

inline ElementData GetElementData(ElementType type)
{
	switch (type)
	{
		// Periode 1

		case ElementType::Hydrogen:
			return { 1.0, 0, 1 };


		// Periode 2

		case ElementType::Carbon:
			return { 12.0, 0, 4 };

		case ElementType::Oxygen:
			return { 16.0, 2, 2 };
	}


	// Niet bereikbaar bij een geldige ElementType,
	// maar de computer waarschuwt anders vooreen mogelijk niet-afgesloten pad.

	return { 0.0, 0, 0 };
}