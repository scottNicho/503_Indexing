#pragma once
#include "NPC.h"
#include<vector>

namespace NCL::CSC8503 {

	class NPC_Group {
	public:

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


		//movement functions
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

			case yellow:
				move(yellowNPCs, forceApplied);
			}
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

	private:

		void move(vector<NPC*>& NPCs, Vector3 force) {
			for (auto xi : NPCs) {
				xi->GetPhysicsObject()->AddForce(force);
			}
		}
	};
}
