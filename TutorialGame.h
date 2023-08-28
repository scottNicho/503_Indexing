#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "ScoreManager.h"
#include "GameManager.h"
#include "Character.h"
#include "Enemy.h"
#include "Door.h"
#include"NPC.h"
#include"NPC_Group.h"

namespace NCL {
	namespace CSC8503 {
		class MenuManager;
		class TutorialGame;
		class Coin :public GameObject {
		public:
			GameWorld* gameWorld;
			Coin(GameWorld* gameWorld);
			void InitCollectableGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius, TutorialGame* game);
			GameObject* AddCollectableToWorld(const Vector3& position, float radius, float inverseMass, MeshGeometry* coinMesh, ShaderBase* basicShader, TextureBase* basicTex);

			void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;
		};


		class TutorialGame		{
		public:
			TutorialGame(CollisionDMethod collMethod);
			~TutorialGame();

			virtual void UpdateGame(float dt);

			MeshGeometry* coinMesh = nullptr;

			TextureBase* basicTex = nullptr;
			ShaderBase* basicShader = nullptr;
			static unsigned int AmountGoats;
		    static bool ExitMain;

		protected:
			NPC_Group* npcGroup;
			float cameraDist = 10.f;
			float myDeltaTime;
			float force;
			float playerRotateSpeed;
			void InitialiseAssets();
		    float minuteTimer;

			void InitCamera();
			void UpdateKeys(float dt);

			void InitWorld();

			

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void CreateMaze(const std::string& filename);

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitCollectableGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void PlayerObjectMovement(float dt);
			void InitiNPCs();

			void BridgeConstraintTest();

			NPC*AddNPCToWorld(const Vector3& position, float radius, float inverseMass, NPC_Colour colour);
			
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCollectableToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddKeyToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddDoorToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			Character* player = nullptr;
			vector<Character*> heard = {};
			Enemy* enemy = nullptr;
			Enemy* goose = nullptr;
			MenuManager* menuManager;
			ScoreManager* scoreManager = nullptr;
			GameManager* gameManager = nullptr;
			Door* door = nullptr;
			Coin* coins = nullptr;
			const unsigned int HeardTimer = 40;
			int HeardRunningTime;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			float damping;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;
			

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	gooseMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* PowerUpObj = nullptr;
			CollisionDMethod collisionMethode;

		};
	}
}

