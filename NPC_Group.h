#pragma once
#include "NPC.h"
#include<vector>
#include<cmath>
#include <ctime>
#include <random>

namespace NCL::CSC8503 {

	class NPC_Group {
	public:

		NPC_Group(Vector2 &dimensions){
			float minDist = std::min(dimensions.x,dimensions.y);
			areanaSize = minDist / 2;
		}

		void createNPC(NPC_Colour colour) {
			NPC* newNPC = new NPC(colour);
			createdNPCs.push_back(newNPC);
		}

		void allocateNPC() {//reallocates NPC's from the created NPC's to other containers depending on colour
			for (auto xi : createdNPCs) {
				NPC_Colour npcColour = xi->getNPC_colour();
				switch (npcColour) {
				case blue:
					blueNPCs.push_back(xi);
					break;
				case red:
					redNPCs.push_back(xi);
					break;
				case green:
					greenNPCs.push_back(xi);
					break;
				case yellow:
					yellowNPCs.push_back(xi);
					break;
				}

			}
			createdNPCs.clear();//all items should have been copied into the appropriate container now
		}

		void createNPC_Group(NPC_Colour setColour, unsigned int numNPCs) {
			if (numNPCs == 0) { return; }
			for (int i = 0; i < numNPCs; i++) {
				createNPC(setColour);
			}
			allocateNPC();
		}
		//update
		void checkRemovalRequired() {
			if (NPC::getRemovalRequired()) {
				deleteDeadNPCs();
			}
		}

		//set group conflict functions
		void conflictMovement() {
			//insert code that chooses between conflict or movement

		}

		//movement functions
		
		
		void runNPCmovement(float dt,NPC_Colour type) {
			switch (type) {
			case blue:
				runBlueNPCmovement(dt);
				updateDistanceTravled(type,dt);
				break;

			case red:
				runRedNPCmovement(dt);
				updateDistanceTravled(type, dt);
				break;

			case green:
				runGreenNPCmovement(dt);
				updateDistanceTravled(type, dt);
				break;

			case yellow:
				runYellowNPCmovement(dt);
				updateDistanceTravled(type, dt);
				break;
			 }

		}

		


		//getters, setters and toggles
		void setBlueCircle(Vector2 circleCentre) {
			blueCircle = circleCentre;
		}

		void setRedCircle(Vector2 circleCentre) {
			redCircle = circleCentre;
		}

		void setGreenCircle(Vector2 circleCentre) {
			greenCircle = circleCentre;
		}

		void setYellowCircle(Vector2 circleCentre) {
			yellowCircle = circleCentre;
		}

		

	protected:
		vector<NPC*> createdNPCs;
		vector<NPC*> blueNPCs;
		vector<NPC*> redNPCs;
		vector<NPC*> greenNPCs;
		vector<NPC*> yellowNPCs;

		bool blueConflict;
		bool redConflict;
		bool greenConflict;
		bool yellowConflict;

		Vector3 blueCircle;
		Vector3 redCircle;
		Vector3 greenCircle;
		Vector3 yellowCircle;

		float areanaSize;

		float blueAngle;
		float redAngle;
		float greenAngle;
		float yellowAngle;

		float blueCircleHD;
		float redCircleHD;
		float greenCircleHD;
		float yellowCircleHD;

		bool blueHDpast = false;
		bool redHDpast = false;
		bool greenHDpast = false;
		bool yellowHDpast = false;

		float blueMovementTime;
		float redMovementTime;
		float greenMovementTime;
		float yellowMovementTime;

		float blueDistanceT;
		float redDistanceT;
		float greenDistanceT;
		float yellowDistanceT;


		const float TIME_LIMIT = 7.0f;

	private:

		

		void setHalfDistance(NPC_Colour type) {
			switch (type) {
			case blue:
				if (blueNPCs.size()>0) {
					blueCircleHD = (((blueNPCs[0]->getXZcoordinates()) - (blueCircle)).Length()) / 2;
				}
				break;

			case red:
				if (redNPCs.size() > 0) {
					redCircleHD = (((redNPCs[0]->getXZcoordinates()) - (redCircle)).Length()) / 2;
				}
				break;

			case green:
				if (greenNPCs.size() > 0) {
					greenCircleHD = (((greenNPCs[0]->getXZcoordinates()) - (greenCircle)).Length()) / 2;
				}
				break;
			case yellow:
				if (yellowNPCs.size() > 0) {
					yellowCircleHD = (((yellowNPCs[0]->getXZcoordinates()) - (yellowCircle)).Length()) / 2;
				}
				break;
			}
		}

		void setRandomCircleCentre(NPC_Colour type) {
			Vector3 circle;

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dist(-areanaSize, areanaSize);

			circle.x = dist(gen);
			circle.y = 1.0f; //we are only considering movement in a 2d palne but using a 3d vector
			circle.z = dist(gen);

			if (int(circle.x) % 2 == 0) {
				circle.x = -circle.x;
			}
			if (int(circle.y) % 3 == 0) {
				circle.y = -circle.y;
			}

			switch (type) {
			case blue:
				blueCircle = circle;
				break;
			case red:
				redCircle = circle;
				break;
			case green:
				greenCircle = circle;
				break;
			case yellow:
				yellowCircle = circle;
				break;
			}
		}

		void deleteDeadNPCs() {
			for (auto xi : blueNPCs) {
				if (!(xi->getAlive())) {
					delete xi;
				}
			}
			for (auto xi : redNPCs) {
				if (!(xi->getAlive())) {
					delete xi;
				}
			}
			for (auto xi : greenNPCs) {
				if (!(xi->getAlive())) {
					delete xi;
				}
			}
			for (auto xi : yellowNPCs) {
				if (!(xi->getAlive())) {
					delete xi;
				}
			}
			NPC::toggleRemovalRequired();
		}


		void updateDistanceTravled(NPC_Colour type, float dt) {
			switch (type) {
			case blue:
				if (blueNPCs.size()>0) {
					blueDistanceT = dt * (blueNPCs[0]->GetPhysicsObject()->GetLinearVelocity()).Length();
					if (blueDistanceT > blueCircleHD) { blueHDpast = true;}
					if(blueDistanceT > 2*blueCircleHD || blueMovementTime >= TIME_LIMIT){
						blueHDpast = false;
						blueMovementTime = 0.0f;
						setRandomCircleCentre(blue);
					}
				}
				break;

			case red:
				if (redNPCs.size() > 0) {
					redDistanceT = dt * (redNPCs[0]->GetPhysicsObject()->GetLinearVelocity()).Length();
					if (redDistanceT > redCircleHD) { redHDpast = true; }
					if (redDistanceT > 2 * redCircleHD || redMovementTime >= TIME_LIMIT) {
						redHDpast = false;
						redMovementTime = 0.0f;
						setRandomCircleCentre(red);
					}
				}
				break;

			case green:
				if (greenNPCs.size() > 0) {
					greenDistanceT = dt * (greenNPCs[0]->GetPhysicsObject()->GetLinearVelocity()).Length();
					if (greenDistanceT > greenCircleHD) { greenHDpast = true; }
					if (greenDistanceT > 2 * greenCircleHD || greenMovementTime >= TIME_LIMIT) {
						greenHDpast = false;
						greenMovementTime = 0.0f;
						setRandomCircleCentre(green);
					}
				}
				break;
			case yellow:
				if (yellowNPCs.size() > 0) {
					yellowDistanceT = dt * (yellowNPCs[0]->GetPhysicsObject()->GetLinearVelocity()).Length();
					if (yellowDistanceT > yellowCircleHD) { yellowHDpast = true;}
					if (yellowDistanceT > 2 * yellowCircleHD || yellowMovementTime >= TIME_LIMIT) {
						yellowHDpast = false;
						yellowMovementTime = 0.0f;
						setRandomCircleCentre(yellow);
					}
				}
				break;
			}
		}

		//movement functions

		void move(vector<NPC*>& NPCs, Vector3 force) {
			for (auto xi : NPCs) {
				xi->GetPhysicsObject()->AddForce(force);
			}
		}

		void moveNPCs(NPC_Colour type, Vector3 forceApplied) {
			switch (type) {
			case blue:
				move(blueNPCs, forceApplied);
				break;

			case red:
				move(redNPCs, forceApplied);
				break;

			case green:
				move(greenNPCs, forceApplied);
				break;

			case yellow:
				move(yellowNPCs, forceApplied);
				break;
			}
		}

		void runBlueNPCmovement(float dt) {
			float cosAng = cos(blueAngle);
			float sinAng = sin(blueAngle);
			Matrix3 rotationMatrix;
			rotationMatrix.SetRow(0, Vector3(cosAng, 0.0f, -sinAng));
			rotationMatrix.SetRow(1, Vector3(0.0f, 1.0f, 0.0f));
			rotationMatrix.SetRow(2, Vector3(sinAng, 0.0f, cosAng));
			Vector3 directionVector = rotationMatrix * blueCircle;
			moveNPCs(blue, directionVector);
			blueMovementTime += dt;
			if (blueHDpast) {
				blueAngle -= dt;
			}
			else
			{
				blueAngle += dt;
			}
		}

		void runRedNPCmovement(float dt) {
			float cosAng = cos(redAngle);
			float sinAng = sin(redAngle);
			Matrix3 rotationMatrix;
			rotationMatrix.SetRow(0, Vector3(cosAng, 0.0f, -sinAng));
			rotationMatrix.SetRow(1, Vector3(0.0f, 1.0f, 0.0f));
			rotationMatrix.SetRow(2, Vector3(sinAng, 0.0f, cosAng));
			Vector3 directionVector = rotationMatrix * redCircle;
			moveNPCs(red, directionVector);
			redMovementTime += dt;
			if (redHDpast) {
				redAngle -= dt;
			}
			else
			{
				redAngle += dt;
			}
		}

		void runGreenNPCmovement(float dt) {
			float cosAng = cos(greenAngle);
			float sinAng = sin(greenAngle);
			Matrix3 rotationMatrix;
			rotationMatrix.SetRow(0, Vector3(cosAng, 0.0f, -sinAng));
			rotationMatrix.SetRow(1, Vector3(0.0f, 1.0f, 0.0f));
			rotationMatrix.SetRow(2, Vector3(sinAng, 0.0f, cosAng));
			Vector3 directionVector = rotationMatrix * greenCircle;
			moveNPCs(green, directionVector);
			greenMovementTime += dt;
			if (greenHDpast) {
				greenAngle -= dt;
			}
			else
			{
				greenAngle += dt;
			}
		}

		void runYellowNPCmovement(float dt) {
			float cosAng = cos(yellowAngle);
			float sinAng = sin(yellowAngle);
			Matrix3 rotationMatrix;
			rotationMatrix.SetRow(0, Vector3(cosAng, 0.0f, -sinAng));
			rotationMatrix.SetRow(1, Vector3(0.0f, 1.0f, 0.0f));
			rotationMatrix.SetRow(2, Vector3(sinAng, 0.0f, cosAng));
			Vector3 directionVector = rotationMatrix * yellowCircle;
			moveNPCs(yellow, directionVector);
			redMovementTime += dt;
			if (yellowHDpast) {
				yellowAngle -= dt;
			}
			else
			{
				yellowAngle += dt;
			}
		}
	};
}
