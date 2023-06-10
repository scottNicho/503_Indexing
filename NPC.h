#pragma once
#include"GameObject.h"
#include"PhysicsObject.h"
#include"RenderObject.h"
#include "Ray.h"
#include "GameWorld.h"

enum NPC_Colour {
	blue,
	red,
	green,
	yellow
};

namespace NCL::CSC8503 {
	using namespace Maths;
	class NPC : public GameObject {
	public:
		NPC(NPC_Colour startColour) :life(100) {
			NPC_type = startColour;
			NPC_World = new GameWorld();
		}

		~NPC() {
			delete NPC_World;
		}

		void AssignID() {
			ID = currentID;
			currentID++;
		}

		void TakeDamage() {
			life -= DAMAGE;
			if (life <= 0) {
				alive = false;
				removalRequired = true;
				this->isActive = false;//?????????????????????????????????????????????????????????? sus
			}
		}

		void setNPCColour(Vector4 newColour) {
			this->GetRenderObject()->SetColour(newColour);
		}

		virtual void OnCollisionBegin(NPC* otherObject) {
			otherObject->TakeDamage();
		}
		//update
		void conflictAction(Vector3 enemyDir) {
			if (!inConflict) { return; }
			float scale = (this->GetTransform().GetScale()).Length();
			PhysicsObject* NPC_Physics = this->GetPhysicsObject();
			Vector3 NPC_Pos = this->GetTransform().GetPosition();
			Vector3 StartingDir = (enemyDir - NPC_Pos).Normalised();
			Vector3 startingPoint = NPC_Pos + (StartingDir * scale);
			RayCollision collisionPoint;
			Ray ray(startingPoint, enemyDir);
			NPC_World->Raycast(ray,collisionPoint);
			if (collisionPoint.node) {
				NPC_Colour hitType	= ((NPC*)collisionPoint.node)->GetNPCtype();
				if (hitType == this->NPC_type || hitType == NULL) { return; }
				else
				{
					Vector3 forceVector = StartingDir * 20;
					NPC_Physics->AddForce(forceVector);
				}
			}
		}


		//getters and setters and toggles
		const NPC_Colour &getNPC_colour() const{
			return NPC_type;
		}

		void toggleAlive() {
			alive = !alive;
		}

		const bool &getAlive()const {
			return alive;
		}

		static const bool &getRemovalRequired(){
			return removalRequired;
		}

		static void toggleRemovalRequired() {
			removalRequired = !removalRequired;
		}

		void setXZcoordinates() {
			XZcoordinates.x = this->GetTransform().GetPosition().x;
			XZcoordinates.y = this->GetTransform().GetPosition().y;
		}

		Vector2& getXZcoordinates(){
			return XZcoordinates;
		}

		const NPC_Colour& GetNPCtype() { return NPC_type; }

	protected:
		static int currentID;
		int ID;
		int life;
		bool alive = true;
		bool inConflict = false; 
		NPC_Colour NPC_type;
		int NPC_ID;
		const int DAMAGE = 10;
		static bool removalRequired;
		Vector2 XZcoordinates;
		GameWorld* NPC_World;
	};

	int NPC::currentID = 0;
	bool NPC::removalRequired = false;
}
