#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"
#include "PositionConstraint.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();
			void InitWall();
			void InitDoor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void BridgeConstraintTest();
			void InitCharacter(const Vector3& position, float radius, float inverseMass = 10.0f);

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, std::string name = "");

			GameObject* AddGoatToWorld(const Vector3& position);
			StateGameObject* AddEnemyToWorld(const Vector3& position);
			void AddBonusToWorld();
			GameObject* AddWallToWorld(const Vector3& position, Vector3 dimensions);
			void AddDoorToWorld(Vector3 startpos, Vector3 doorpos, Vector3 size,int i);
			void DoorConstraint(int i);
			void CatchGoat();
			void AddHurdleToWorld(const Vector3& position);
			void PhysicsUpdate();

#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* charMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 20, 0);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			StateGameObject* AddStateObjectToWorld(const Vector3& position, bool vert);
			StateGameObject* simpleEnemy[2];

			GameObject* characterRole;
			GameObject* goat;
			GameObject* hurdle;
			GameObject* door[4][2];
			GameObject* bonus[4];
			PositionConstraint* hook;
			PositionConstraint * doorLink[4];
		};
	}
}