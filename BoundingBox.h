#pragma once
#include"Vector2.h"
#include"GameObject.h"
#include <vector>

class BoundingBox
{
public:
	BoundingBox(Vector2& BLeft, Vector2& TRight) :minLeftPoint(BLeft), topRightPoint(TRight) {}

	const Vector2& getBottomLeft() {
		return minLeftPoint;
	}

	const Vector2& getTopRight() {
		return minLeftPoint;
	}

	Vector2 GetCentre() {
		return(minLeftPoint + topRightPoint) * 0.5;
	}

	float MinimumSizeIncrease(BoundingBox newBox) {
		float increaseSize = 0.0f;
		if (newBox.topRightPoint.x > topRightPoint.x) {
			increaseSize += abs(newBox.topRightPoint.x - topRightPoint.x);
		}
		else if (newBox.minLeftPoint.x < minLeftPoint.x) {
			increaseSize += abs(newBox.minLeftPoint.x - minLeftPoint.x);
		}

		if (newBox.topRightPoint.y > topRightPoint.y) {
			increaseSize += abs(newBox.topRightPoint.y - topRightPoint.y);
		}
		else if (newBox.minLeftPoint.y < minLeftPoint.y) {
			increaseSize += abs(newBox.minLeftPoint.y - minLeftPoint.y);
		}

		return increaseSize;
	}

private:

	Vector2 minLeftPoint;
	Vector2 topRightPoint;

};