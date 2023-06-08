#pragma once
#include"GameObject.h"
#include"PhysicsObject.h"
#include"RenderObject.h"

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
		NPC(NPC_Colour startColour) :life(10) {
			NPC_type = startColour;
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

	protected:
		static int currentID;
		int ID;
		int life;
		bool alive = true;
		NPC_Colour NPC_type;
		int NPC_ID;
		const int DAMAGE = 10;
		static bool removalRequired;
	};

	int NPC::currentID = 0;
	bool NPC::removalRequired = false;
}
