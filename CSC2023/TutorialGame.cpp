#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "OrientationConstraint.h"
#include "StateGameObject.h"



using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = true;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("cube.msh");//"apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		//world->GetMainCamera().SetPitch(angles.x);
		//world->GetMainCamera().SetYaw(angles.y);
	}

	UpdateKeys();
	physics->UseGravity(useGravity);

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	if (testStateObject) {
		testStateObject->Update(dt);
	}

	//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	
	PhysicsUpdate();

	int time = 120 - physics->gameRun->GetTime();
	Debug::Print("Time:" + std::to_string(time), Vector2(5, 5), Debug::RED);
	if (physics->gameRun->GetOntime() == 1) {
		physics->gameRun->SetTime(dt);
	}

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		//physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	/*if (lockedObject) {
		LockedObjectMovement();
	}
	else {*/
	DebugObjectMovement();
	//}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
		characterRole->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		characterRole->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
		characterRole->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		characterRole->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		characterRole->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		characterRole->GetPhysicsObject()->AddTorque(Vector3(0, 0, -10));
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		characterRole->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		characterRole->GetPhysicsObject()->AddTorque(Vector3(0, 0, 10));
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(700.0f);
	world->GetMainCamera().SetPitch(-90.0f);
	world->GetMainCamera().SetYaw(0.0f);
	world->GetMainCamera().SetPosition(Vector3(0, 500, 0));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	InitDefaultFloor();
	InitWall();
	InitCharacter(Vector3(-150, 6, 150), 6.0f);
	//InitCharacter(Vector3(150, 6, 130), 6.0f);
	AddGoatToWorld(Vector3(150, 5, -150));

	AddCubeToWorld(Vector3(-140, -0.8, 150), Vector3(20, 1, 10), 0, "start");
	AddCubeToWorld(Vector3(150, -0.9, 140), Vector3(10, 1, 20), 0, "button");
	AddCubeToWorld(Vector3(-40, 10, 150), Vector3(0.1, 10, 10), 0, "wall");
	AddHurdleToWorld(Vector3(130, 10, 150));
	//AddSphereToWorld(Vector3(-120, 6, -80), 6.0f);
	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	//InitGameExamples();
	//BridgeConstraintTest();

	InitDoor();
	AddBonusToWorld();
	Vector3 cube = Vector3(8, 8, 8);
	AddCubeToWorld(Vector3(-60, 8, -140), cube, 1);
	AddCubeToWorld(Vector3(-40, 8, -100), cube,  1);
	AddCubeToWorld(Vector3(-20, 8, -120), cube, 1);
	AddCubeToWorld(Vector3(20, 8, -140), cube,  1);
	AddCubeToWorld(Vector3(-20, 8, -160), cube, 1);

	testStateObject = AddStateObjectToWorld(Vector3(50, 5, -40),false);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddWallToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* wall = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	wall->SetBoundingVolume((CollisionVolume*)volume);
	wall->GetTransform().SetScale(dimensions * 2).SetPosition(position);

	wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
	wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

	wall->GetPhysicsObject()->SetInverseMass(0);
	wall->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(wall);

	return wall;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, std::string name) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, nullptr, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	cube->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	cube->SetName(name);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddGoatToWorld(const Vector3& position) {
	float meshSize = 5.0f;
	float inverseMass = 5.0f;

	goat = new GameObject();
	AABBVolume* volume = new AABBVolume(Vector3(meshSize, meshSize, meshSize));
	//SphereVolume* volume = new SphereVolume(meshSize);

	goat->SetBoundingVolume((CollisionVolume*)volume);

	goat->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	goat->SetRenderObject(new RenderObject(&goat->GetTransform(), charMesh, nullptr, basicShader));
	goat->SetPhysicsObject(new PhysicsObject(&goat->GetTransform(), goat->GetBoundingVolume()));
	goat->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));

	goat->GetPhysicsObject()->SetInverseMass(inverseMass);
	goat->GetPhysicsObject()->InitSphereInertia();
	goat->SetName("goat");

	world->AddGameObject(goat);

	return goat;
}

StateGameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 10.0f;
	float inverseMass = 0.5f;

	StateGameObject* character = new StateGameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	character->SetName("enemy");

	world->AddGameObject(character);

	return character;
}

void TutorialGame::AddBonusToWorld() {
	Vector3 position[4] = { Vector3(-30,5,110),Vector3(-150,5,10),Vector3(150,5,0),Vector3(110,5,-50) };
	for (int i = 0; i < 4; i++) {
		bonus[i] = new GameObject();
		std::string name = "bonus" + std::to_string(i);
		SphereVolume* volume = new SphereVolume(5);
		bonus[i]->SetBoundingVolume((CollisionVolume*)volume);
		bonus[i]->GetTransform().SetScale(Vector3(5, 5, 5)).SetPosition(position[i]);

		bonus[i]->SetRenderObject(new RenderObject(&bonus[i]->GetTransform(), sphereMesh, nullptr, basicShader));
		bonus[i]->SetPhysicsObject(new PhysicsObject(&bonus[i]->GetTransform(), bonus[i]->GetBoundingVolume()));

		bonus[i]->GetPhysicsObject()->SetInverseMass(0);
		bonus[i]->GetPhysicsObject()->InitSphereInertia();
		bonus[i]->SetName(name);
		bonus[i]->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));

		world->AddGameObject(bonus[i]);
	}
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position, bool vert) {
	float meshSize = 5.0f;
	float inverseMass = 0.5f;

	StateGameObject* character = new StateGameObject();

	SphereVolume* volume = new SphereVolume(meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), sphereMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->SetVert(vert);
	character->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
	character->SetName("enemy");

	world->AddGameObject(character);

	return character;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitCharacter(const Vector3& position, float radius, float inverseMass) {
	characterRole = new GameObject();
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	characterRole->SetBoundingVolume((CollisionVolume*)volume);

	characterRole->GetTransform().SetScale(sphereSize).SetPosition(position);

	characterRole->SetRenderObject(new RenderObject(&characterRole->GetTransform(), sphereMesh, basicTex, basicShader));
	characterRole->SetPhysicsObject(new PhysicsObject(&characterRole->GetTransform(), characterRole->GetBoundingVolume()));

	characterRole->GetPhysicsObject()->SetInverseMass(inverseMass);
	characterRole->GetPhysicsObject()->InitSphereInertia();
	characterRole->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	characterRole->SetName("role");

	world->AddGameObject(characterRole);
}

void TutorialGame::InitWall() {
	Vector3 cubeDims = Vector3(20, 20, 20);
	Vector3 position;
	for (int x = 0; x < 10; ++x) {
		position = Vector3(-180 + x * 40, 20, -180);
		AddWallToWorld(position, cubeDims);
		AddWallToWorld(Vector3(position.x, 20, 180), cubeDims);
	}
	for (int z = 0; z < 8; ++z) {
		position = Vector3(-180, 20, -140 + z * 40);
		AddWallToWorld(position, cubeDims);
		AddWallToWorld(Vector3(180, 20, position.z), cubeDims);
	}
	cubeDims = Vector3(10, 10, 10);
	for (int i = 0; i < 4; ++i) {
		position = Vector3(-110, 10, 150 - i * 20);
		AddWallToWorld(position, cubeDims);
		AddWallToWorld(Vector3(-70, 10, position.z - 180), cubeDims);
		AddWallToWorld(Vector3(-50, 10, position.z), cubeDims);
		AddWallToWorld(Vector3(50, position.y, position.z - 80), cubeDims);
		AddWallToWorld(Vector3(70, position.y, position.z - 20), cubeDims);
		AddWallToWorld(Vector3(90, position.y, position.z - 160), cubeDims);
		AddWallToWorld(Vector3(130, position.y, position.z - 80), cubeDims);
		AddWallToWorld(Vector3(130, position.y, position.z - 160), cubeDims);
	}
	for (int i = 0; i < 3; ++i) {
		position = Vector3(-150 + i * 20, 10, 30);
		AddWallToWorld(position, cubeDims);
		AddWallToWorld(Vector3(position.x + 60, 10, 30), cubeDims);
		AddWallToWorld(Vector3(position.x + 120, 10, 30), cubeDims);
		AddWallToWorld(Vector3(position.x + 160, 10, 130), cubeDims);
		AddWallToWorld(Vector3(position.x + 240, 10, 130), cubeDims);
		AddWallToWorld(Vector3(position.x + 40, 10, -110), cubeDims);
		AddWallToWorld(Vector3(position.x + 100, 10, -70), cubeDims);
		AddWallToWorld(Vector3(position.x + 160, 10, -70), cubeDims);
		AddWallToWorld(Vector3(position.x + 180, 10, -10), cubeDims);
		AddWallToWorld(Vector3(position.x + 200, 10, -130), cubeDims);
	}
	AddWallToWorld(Vector3(-150, 10, 110), cubeDims);
	AddWallToWorld(Vector3(-130, 10, 50), cubeDims);
	AddWallToWorld(Vector3(-130, 10, -110), cubeDims);
	AddWallToWorld(Vector3(-130, 10, -130), cubeDims);
	AddWallToWorld(Vector3(-90, 10, -150), cubeDims);
	AddWallToWorld(Vector3(-30, 10, 130), cubeDims);
	AddWallToWorld(Vector3(-30, 10, 10), cubeDims);
	AddWallToWorld(Vector3(-30, 10, -10), cubeDims);
	AddWallToWorld(Vector3(10, 10, 50), cubeDims);
	AddWallToWorld(Vector3(10, 10, 70), cubeDims);
	AddWallToWorld(Vector3(10, 10, -90), cubeDims);
	AddWallToWorld(Vector3(10, 10, -110), cubeDims);
	AddWallToWorld(Vector3(70, 10, -70), cubeDims);
	AddWallToWorld(Vector3(90, 10, 70), cubeDims);
	AddWallToWorld(Vector3(90, 10, 30), cubeDims);
	AddWallToWorld(Vector3(90, 10, -130), cubeDims);
	AddWallToWorld(Vector3(90, 10, -150), cubeDims);
	AddWallToWorld(Vector3(110, 10, 30), cubeDims);
	AddWallToWorld(Vector3(130, 10, 90), cubeDims);
	AddWallToWorld(Vector3(130, 10, 110), cubeDims);
}

void TutorialGame::InitGameExamples() {
	AddGoatToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
		if (lockedObject == characterRole) {
			InitCamera();
		}
		else {
			lockedObject = characterRole;
		}
	}
	if (inSelectionMode) {
		Debug::Print("select mode", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				//selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				//selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		Debug::Print("camera mode", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
		if (closestCollision.node == selectionObject) {
			selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(100, 300, 100);
	//Vector3 startPos = Vector3(500, 500, 500);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::InitDoor() {
	AddDoorToWorld(Vector3(10, 10, 110), Vector3(0, 0, -20), Vector3(3, 10, 10),0);
	AddDoorToWorld(Vector3(-150, 10, -50), Vector3(20, 0, 0), Vector3(10, 10, 3),1);
	AddDoorToWorld(Vector3(-90, 10, -50), Vector3(-20, 0, 0), Vector3(10, 10, 3),2);
	AddDoorToWorld(Vector3(90, 10, -110), Vector3(0, 0, 20), Vector3(3, 10, 10),3);
}

void TutorialGame::AddDoorToWorld(Vector3 startpos, Vector3 doorpos, Vector3 size,int i) {
	Vector3 cubeSize = Vector3(10, 10, 10);
	Vector3 doorSize = size;
	float cubeDistance = 20; // distance between links
	Vector3 startPos = startpos;
	std::string name = "door" + std::to_string(i);
	door[i][0] = AddWallToWorld(startPos, cubeSize);
	door[i][1] = AddCubeToWorld(startPos + doorpos, doorSize, 0,name);
}

void TutorialGame::CatchGoat() {
	float maxDistance = 20; // constraint distance
	hook = new PositionConstraint(characterRole, goat, maxDistance);
	world->AddConstraint(hook);
}

void TutorialGame::AddHurdleToWorld(const Vector3& position) {
	hurdle = new GameObject();
	Vector3 cubeSize = Vector3(10, 10, 10);
	AABBVolume* volume = new AABBVolume(cubeSize);
	hurdle->SetBoundingVolume((CollisionVolume*)volume);

	hurdle->GetTransform().SetPosition(position).SetScale(cubeSize * 2);

	hurdle->SetRenderObject(new RenderObject(&hurdle->GetTransform(), cubeMesh, basicTex, basicShader));
	hurdle->SetPhysicsObject(new PhysicsObject(&hurdle->GetTransform(), hurdle->GetBoundingVolume()));

	hurdle->GetPhysicsObject()->SetInverseMass(0);
	hurdle->GetPhysicsObject()->InitCubeInertia();

	hurdle->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	hurdle->SetName("cube");

	world->AddGameObject(hurdle);
}

void TutorialGame::DoorConstraint(int i) {
	door[i][1]->GetPhysicsObject()->SetInverseMass(5);
	doorLink[i] = new PositionConstraint(door[i][0], door[i][1], 25);
	world->AddConstraint(doorLink[i]);
}

void TutorialGame::PhysicsUpdate() {
	if (physics->gameRun->GetCatchgoat() != physics->gameRun->GetHookgoat()) {
		if (physics->gameRun->GetCatchgoat() == 1) {
			goat->GetPhysicsObject()->SetInverseMass(5);
			CatchGoat();
			physics->gameRun->SetHookgoat(1);
		}
		else {
			world->RemoveConstraint(hook, true);
			goat->GetPhysicsObject()->SetInverseMass(0.1);
			hurdle->GetPhysicsObject()->SetInverseMass(1);
			physics->gameRun->SetHookgoat(0);
		}
	}
	for (int i = 0; i < 4; i++) {
		if (physics->gameRun->GetLockdoor(i) == 1) {
			DoorConstraint(i);
			physics->gameRun->SetLockdoor(i, 2);
		}
		if (physics->gameRun->GetLockdoor(i) == 3) {
			world->RemoveConstraint(doorLink[i], true);
			physics->gameRun->SetLockdoor(i, 4);
		}
	}
	int obj = 4;
	for (int i = 0; i < 4; i++) {
		if (physics->gameRun->bonus[i] == 1) {
			obj -= 1;
			if (bonus[i]!=nullptr) {
				world->RemoveGameObject(bonus[i]);
				bonus[i] = nullptr;
				physics->gameRun->AddScore((i + 1) * 10);
			}
		}
	}
	Debug::Print("Score:" + std::to_string(physics->gameRun->GetScore()), Vector2(50, 5), Debug::RED);
	Debug::Print("Destroyable:" + std::to_string(obj), Vector2(70, 5), Debug::RED);
}