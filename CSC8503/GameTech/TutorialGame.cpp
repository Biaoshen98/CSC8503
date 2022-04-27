#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "..//CSC8503Common/PositionConstraint.h"

#include "../CSC8503Common/StateGameObject.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
	machine = new PushdownMachine(new IntroScreen(this));
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	//useGravity		= false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	//InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("Male1.msh", &charMeshA);
	loadFunc("courier.msh", &charMeshB);
	loadFunc("security.msh", &enemyMesh);
	loadFunc("coin.msh", &bonusMesh);
	loadFunc("capsule.msh", &capsuleMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	floorTex = (OGLTexture*)TextureLoader::LoadAPITexture("floor.png");
	wallTex = (OGLTexture*)TextureLoader::LoadAPITexture("wall.png");
	smallwallTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	//InitWorld();//when initgame1 and initgame2 func done, donot call this func
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;
	delete basicTex;
	delete basicShader;
	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	glClearColor(1, 1, 1, 1);
	if (dt == 0) {
		renderer->DrawString("Now Game Paused(ESC)", Vector2(30, 10));
	}
	else {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}
		/*
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}
	*/
		UpdateKeys();
		SelectObject();
		MoveSelectedObject();
		physics->Update(dt);
		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset+Vector3(0,50,0);

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera()->SetPosition(camPos);
			world->GetMainCamera()->SetPitch(angles.x);
			world->GetMainCamera()->SetYaw(angles.y);

			
		}

	}
	//state machine code begin
	if (testStateObject) {
		testStateObject->Update(dt);
		testStateObject1->Update(dt);
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);
	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitGameWorld1(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject = nullptr;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		InitGameWorld2(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		//useGravity = !useGravity; //Toggle gravity!
		//physics->UseGravity(useGravity);
	//}
	physics->UseGravity(true);
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	//world->GetMainCamera()->SetPosition(Vector3(500, 500, 500));//test for constrains and solvers
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);//create rand cube and sphere
	InitGameExamples();
	InitDefaultFloor();
	//Bridge();//!!!s
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));//state machine code
	//testStateObject1 = AddStateObjectToWorld(Vector3(0, 20, 0));//state machine code
}

void TutorialGame::BridgeConstraintTest() {
	GameObject* cube = new GameObject("jumppad");

	Vector3 cubeSize = Vector3(1, 0.2, 3);
	float invCubeMass = 3; //how heavy the middle pieces are
	int numLinks = 13;
	float maxDistance = 2; // constraint distance
	float cubeDistance = 2; // distance between links

	Vector3 startPos = Vector3(60, 5, 95);
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
/*
A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("jumppad");

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, floorTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
//add wall to world

GameObject* TutorialGame::AddWallBeforeAndAfterToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(1, 10, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, wallTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}//add wall to world
GameObject* TutorialGame::AddWallLeftAndRightToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(100, 10, 1);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, wallTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	//floor->GetPhysicsObject()->set

	world->AddGameObject(floor);

	return floor;
}//add wall to world

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

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
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

void TutorialGame::ImpulseResolveCollision(GameObject& a, GameObject& b) const {
	if ("Win" &&"sphereplayer") {
		renderer->DrawString("Win the game!!!", Vector2(30, 30));
		return;
	}
}

void TutorialGame::InitAccelerationBlockWorld() {
	AddAccelerationBlock1ToWorld(Vector3(95, 0.1, 55));
	AddAccelerationBlock2ToWorld(Vector3(0, 0.1, 70));
	AddAccelerationBlock3ToWorld(Vector3(-50, 0.1, -70));
	AddAccelerationBlock4ToWorld(Vector3(0, 0.1, 0));//jump!
	AddAccelerationBlock5ToWorld(Vector3(50, 0.1, -70));
}
/// <summary>
/// jiajiajia
/// </summary>
void TutorialGame::ExWallAndFloorforGame1() {
	AddFloor3ToWorld(Vector3(0, -5, 0));
	AddWallBeforeAndAfter3ToWorld(Vector3(1000, 10, 0));//wall top
	AddWallBeforeAndAfter3ToWorld(Vector3(-1000, 10, 0));//wall bottom
	AddWallLeftAndRight3ToWorld(Vector3(0, 10, 1000));//wall right
	AddWallLeftAndRight3ToWorld(Vector3(0, 10, -1000));//wall left
}
//daozhe!
void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
	AddWallBeforeAndAfterToWorld(Vector3(100, 10, 0));//wall top
	AddWallBeforeAndAfterToWorld(Vector3(-100, 10, 0));//wall bottom
	AddWallLeftAndRightToWorld(Vector3(0, 10, 100));//wall right
	AddWallLeftAndRightToWorld(Vector3(0, 10, -100));//wall left
	AddMiddle1ToWorld(Vector3(90, 1, 62));
	AddMiddle2ToWorld(Vector3(0, 1, -62));
	AddMiddle3ToWorld(Vector3(-50, 1, 62));
	AddMiddle4ToWorld(Vector3(60, 1, 62));

}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(95, 5, 95));
	AddEnemyToWorld(Vector3(-95, 5, 95));
	//AddWinPointToWorld(Vector3(-95, 5, 95));
	//AddBonusToWorld(Vector3(10, 5, 0));
	//AddCapsuleToWorld(Vector3(50, 50, 50), 10.0f, 8.0f);
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject("player");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 5.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject("Enemy");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(3, 0.25, 1))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {//state machine code
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(3, 0.25, 1))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	StateGameObject* apple = new StateGameObject();

	//SphereVolume* volume = new SphereVolume(0.25f);
	AABBVolume* volume = new AABBVolume(dimensions);

	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(dimensions)
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, wallTex, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(inverseMass);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
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
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));
		renderer->DrawString("Press M to go back to menu!", Vector2(5, 75));
		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				//add draw information function here
				
				float x = selectionObject->GetTransform().GetPosition().x;
				float y = selectionObject->GetTransform().GetPosition().y;
				float z = selectionObject->GetTransform().GetPosition().z;
				string pos = std::to_string(x);
				pos = pos + ',';
				pos += std::to_string(y);
				pos = pos + ',';
				pos += std::to_string(z);
				renderer->DrawString(pos, Vector2(10, 10));
				string name;
				if (selectionObject->GetName() == "") {
					name = "noName";
					renderer->DrawString(name, Vector2(10, 15));
				}
				else {
					name = selectionObject->GetName();
					renderer->DrawString(name, Vector2(10, 15));
				}
				

				//add draw information function here
				selectionObject = nullptr;
				lockedObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
			    selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
		renderer->DrawString("Press M to go back to menu!", Vector2(5, 75));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if (selectionObject) {
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

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
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	if (!selectionObject) {
		return;
	}
	isSelected = true;
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				//selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);//tu1
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
	//use WASD to move selectedobject code begin
	if (!selectionObject) {
		return;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::TAB))
	{
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SPACE)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::SHIFT)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::Q)) {//lets rotate
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::E)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-1, 0, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::R)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -1, 0) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::J)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, 1) * forceMagnitude * 0.02f);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::L)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 0, -1) * forceMagnitude * 0.02f);
		}

	}
	//use WASD to move selectedobject code end
}

//coursework function begin
void TutorialGame::DrawMenu() {
	glClearColor(0, 0, 0, 1);
	Debug::FlushRenderables(0);
	renderer->DrawString("Welcome to game", Vector2(10, 10));
	renderer->DrawString("Press '1' for Play Football Game", Vector2(10, 20));
	renderer->DrawString("Press '2' for Run and Catch Game", Vector2(10, 30));
	renderer->Render();
	world->ClearAndErase();
	player = nullptr;
	physics->Clear();
	winnerName.clear();
}
void TutorialGame::DrawWin() {
	renderer->DrawString("Wow, you win the game!!!!!!", Vector2(10, 10));
}
void TutorialGame::DrawLose(std::string winnner) {
	renderer->DrawString("Oh, you Dead", Vector2(10, 10));
}
void TutorialGame::DrawPause() {
	renderer->DrawString("now game paused, come back soon!", Vector2(10, 10));
}
void TutorialGame::InitGameWorld1() {//ball
	InitialiseAssets();//add assets first

	world->ClearAndErase();
	physics->Clear();
	//InitGameExamples();
	InitDefaultFloor();
	BridgeConstraintTest();//!!!s
	InitAccelerationBlockWorld();
	AddSlopeToWorld(Vector3(-85, 1, 55));
	AddSpherePlayerToWorld(Vector3(95, 10, 92), 1, 1);//add player
	AddPlayerToWorld(Vector3(95, 10, 95));
	AddStartToWorld(Vector3(95, 0, 95));
	Vector3 coin = Vector3(-90, 1, 0);
	Vector3 coin1 = Vector3(-90, 1, 0);
	Vector3 coin2 = Vector3(60, 1, 40);
	Vector3 coin3 = Vector3(-60, 1, 40);
	Vector3 coin4 = Vector3(-50, 1, 30);
	Vector3 coin5 = Vector3(40, 1, 50);
	Vector3 coin6 = Vector3(0, 1, 20);
	Vector3 coin7 = Vector3(0, 1, -20);
	Vector3 coin8 = Vector3(0, 1, 40);
	Vector3 coin9 = Vector3(0, 1, -40);
	AddWinPointToWorld(Vector3(-85, 0.1, 85));
	AddMiddle5ToWorld(Vector3(-55, 2, 85));
	AddMiddle6ToWorld(Vector3(-85, 2, 70));
	AddCoinToWorld(coin, "coin");
	AddCoinToWorld(coin1, "coin");
	AddCoinToWorld(coin2, "coin");
	AddCoinToWorld(coin3, "coin");
	AddCoinToWorld(coin4, "coin");
	AddCoinToWorld(coin5, "coin");
	AddCoinToWorld(coin6, "coin");
	AddCoinToWorld(coin7, "coin");
	AddCoinToWorld(coin8, "coin");
	AddCoinToWorld(coin9, "coin");
	ExWallAndFloorforGame1();
	AddSpinObstacleToWorld(Vector3(0, 2, 0));
	testStateObject = AddStateWallToWorld(Vector3(-50, 1, 38), Vector3(3, 3, 3), 3.0f);//state machine code
	testStateObject1 = AddStateWallToWorld(Vector3(87, 0, 38), Vector3(3, 3, 3), 3.0f);//state machine code

}
void TutorialGame::InitGameWorld2() {//maze
	InitialiseAssets();//add assets first
	world->ClearAndErase();
	physics->Clear();
	AddFloor2ToWorld(Vector3(0, -2, 0));
	Vector3 coin = Vector3(-9,1,0);
	Vector3 coin1 = Vector3(9, 1, 0);
	Vector3 coin2 = Vector3(9, 1, 9);
	Vector3 coin3 = Vector3(-9, 1, 4);
	Vector3 coin4 = Vector3(-9, 1, 3);
	Vector3 coin5 = Vector3(9, 1, 1);
	Vector3 coin6 = Vector3(9, 1, 2);
	Vector3 coin7 = Vector3(9, 1, 3);
	Vector3 coin8 = Vector3(9, 1, 4);
	Vector3 coin9 = Vector3(-9, 1, -6);
	AddWallBeforeAndAfter2ToWorld(Vector3(11, 1, 0));//wall top
	AddWallBeforeAndAfter2ToWorld(Vector3(-11, 1, 0));//wall bottom
	AddWallLeftAndRight2ToWorld(Vector3(0, 1, 11));//wall right
	AddWallLeftAndRight2ToWorld(Vector3(0, 1, -11));//wall left
	AddMiddle10ToWorld(Vector3(4, 1, 4));
	AddMiddle9ToWorld(Vector3(0, 1, -4));
	AddMiddle8ToWorld(Vector3(-4, 1, 4));
	AddWinPoint2ToWorld(Vector3(-9, 0.1, 9));
	AddSpherePlayerToWorld(Vector3(9, 10, 9), 1, 1);//add player
	BridgeConstraintTest();//!!!s
	AddCoinToWorld(coin, "coin");
	AddCoinToWorld(coin1, "coin");
	AddCoinToWorld(coin2, "coin");
	AddCoinToWorld(coin3, "coin");
	AddCoinToWorld(coin4, "coin");
	AddCoinToWorld(coin5, "coin");
	AddCoinToWorld(coin6, "coin");
	AddCoinToWorld(coin7, "coin");
	AddCoinToWorld(coin8, "coin");
	AddCoinToWorld(coin9, "coin");
	//AddWinPointToWorld(Vector3(-9, 1, 85));
	AddSpherePlayer1ToWorld(Vector3(-9, 10, -9),1,1);
	ExWallAndFloorforGame1();
	testStateObject = AddStateWallToWorld(Vector3(-4, 100, 1), Vector3(0.25, 0.25, 0.25), 5.0f);//state machine code
	testStateObject1 = AddStateWallToWorld(Vector3(-2, 100, -1), Vector3(0.25, 0.25, 0.25), 5.0f);//state machine code
}
GameObject* TutorialGame::AddJumpPad(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("jumppad");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddSpherePlayerToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphereplayer");

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

GameObject* TutorialGame::AddSpherePlayer1ToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphereplayer1");

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

GameObject* TutorialGame::AddMiddle1ToWorld(const Vector3& position) {
	GameObject* Obstacle1 = new GameObject();

	Vector3 Obstacle1Size = Vector3(2, 8, 75);

	AABBVolume* volume = new AABBVolume(Obstacle1Size / 2);

	Obstacle1->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle1->GetTransform()
		.SetScale(Obstacle1Size)
		.SetPosition(position);

	Obstacle1->SetRenderObject(new RenderObject(&Obstacle1->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle1->SetPhysicsObject(new PhysicsObject(&Obstacle1->GetTransform(), Obstacle1->GetBoundingVolume()));

	Obstacle1->GetPhysicsObject()->SetInverseMass(0);
	Obstacle1->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle1);

	return Obstacle1;
}
GameObject* TutorialGame::AddMiddle2ToWorld(const Vector3& position) {
	GameObject* Obstacle2 = new GameObject();

	Vector3 Obstacle2Size = Vector3(2, 8, 75);

	AABBVolume* volume = new AABBVolume(Obstacle2Size / 2);

	Obstacle2->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle2->GetTransform()
		.SetScale(Obstacle2Size)
		.SetPosition(position);

	Obstacle2->SetRenderObject(new RenderObject(&Obstacle2->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle2->SetPhysicsObject(new PhysicsObject(&Obstacle2->GetTransform(), Obstacle2->GetBoundingVolume()));

	Obstacle2->GetPhysicsObject()->SetInverseMass(0);
	Obstacle2->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle2);

	return Obstacle2;
}
GameObject* TutorialGame::AddMiddle3ToWorld(const Vector3& position) {
	GameObject* Obstacle3 = new GameObject();

	Vector3 Obstacle3Size = Vector3(2, 8, 75);

	AABBVolume* volume = new AABBVolume(Obstacle3Size / 2);

	Obstacle3->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle3->GetTransform()
		.SetScale(Obstacle3Size)
		.SetPosition(position);

	Obstacle3->SetRenderObject(new RenderObject(&Obstacle3->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle3->SetPhysicsObject(new PhysicsObject(&Obstacle3->GetTransform(), Obstacle3->GetBoundingVolume()));

	Obstacle3->GetPhysicsObject()->SetInverseMass(0);
	Obstacle3->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle3);

	return Obstacle3;
}

GameObject* TutorialGame::AddMiddle4ToWorld(const Vector3& position) {
	GameObject* Obstacle3 = new GameObject();

	Vector3 Obstacle3Size = Vector3(2, 8, 75);

	AABBVolume* volume = new AABBVolume(Obstacle3Size / 2);

	Obstacle3->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle3->GetTransform()
		.SetScale(Obstacle3Size)
		.SetPosition(position);

	Obstacle3->SetRenderObject(new RenderObject(&Obstacle3->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle3->SetPhysicsObject(new PhysicsObject(&Obstacle3->GetTransform(), Obstacle3->GetBoundingVolume()));

	Obstacle3->GetPhysicsObject()->SetInverseMass(0);
	Obstacle3->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle3);

	return Obstacle3;
}

GameObject* TutorialGame::AddMiddle5ToWorld(const Vector3& position) {
	GameObject* Obstacle5 = new GameObject();

	Vector3 Obstacle5Size = Vector3(2, 4, 30);

	AABBVolume* volume = new AABBVolume(Obstacle5Size / 2);

	Obstacle5->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle5->GetTransform()
		.SetScale(Obstacle5Size)
		.SetPosition(position);

	Obstacle5->SetRenderObject(new RenderObject(&Obstacle5->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle5->SetPhysicsObject(new PhysicsObject(&Obstacle5->GetTransform(), Obstacle5->GetBoundingVolume()));

	Obstacle5->GetPhysicsObject()->SetInverseMass(0);
	Obstacle5->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle5);

	return Obstacle5;
}

GameObject* TutorialGame::AddMiddle6ToWorld(const Vector3& position) {
	GameObject* Obstacle6 = new GameObject();

	Vector3 Obstacle6Size = Vector3(30, 4, 2);

	AABBVolume* volume = new AABBVolume(Obstacle6Size / 2);

	Obstacle6->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle6->GetTransform()
		.SetScale(Obstacle6Size)
		.SetPosition(position);

	Obstacle6->SetRenderObject(new RenderObject(&Obstacle6->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle6->SetPhysicsObject(new PhysicsObject(&Obstacle6->GetTransform(), Obstacle6->GetBoundingVolume()));

	Obstacle6->GetPhysicsObject()->SetInverseMass(0);
	Obstacle6->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle6);

	return Obstacle6;
}

GameObject* TutorialGame::AddStartToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("jumppad");

	Vector3 stageSize = Vector3(5, 5, 5);

	//AABBVolume* volume = new AABBVolume(wallLRSize);
	AABBVolume* volume = new AABBVolume(stageSize);

	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(stageSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, wallTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
/// <summary>
/// jiajiajia
/// </summary>
/// <param name="position"></param>
/// <returns></returns>
GameObject* TutorialGame::AddSpinObstacleToWorld(const Vector3& position) {
	GameObject* SpinObstacle = new GameObject("SpinObstacle");

	Vector3 SpinObstacleSize = Vector3(20, 2, 1);

	AABBVolume* volume = new AABBVolume(SpinObstacleSize / 2);

	SpinObstacle->SetBoundingVolume((CollisionVolume*)volume);
	SpinObstacle->GetTransform()
		.SetScale(SpinObstacleSize)
		.SetPosition(position);

	SpinObstacle->SetRenderObject(new RenderObject(&SpinObstacle->GetTransform(), cubeMesh, basicTex, basicShader));
	SpinObstacle->SetPhysicsObject(new PhysicsObject(&SpinObstacle->GetTransform(), SpinObstacle->GetBoundingVolume()));

	SpinObstacle->GetPhysicsObject()->SetInverseMass(100);
	SpinObstacle->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(SpinObstacle);

	return SpinObstacle;
}

GameObject* TutorialGame::AddAccelerationBlock1ToWorld(const Vector3& position) {
	GameObject* AccelerationBlock = new GameObject("Block1");

	Vector3 AccelerationBlockSize = Vector3(5, 0, 60);

	AABBVolume* volume = new AABBVolume(AccelerationBlockSize / 2);

	AccelerationBlock->SetBoundingVolume((CollisionVolume*)volume);
	AccelerationBlock->GetTransform()
		.SetScale(AccelerationBlockSize)
		.SetPosition(position);

	AccelerationBlock->SetRenderObject(new RenderObject(&AccelerationBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	AccelerationBlock->SetPhysicsObject(new PhysicsObject(&AccelerationBlock->GetTransform(), AccelerationBlock->GetBoundingVolume()));

	AccelerationBlock->GetPhysicsObject()->SetInverseMass(0);
	AccelerationBlock->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(AccelerationBlock);

	return AccelerationBlock;
}

GameObject* TutorialGame::AddAccelerationBlock2ToWorld(const Vector3& position) {
	GameObject* AccelerationBlock = new GameObject("Block2");

	Vector3 AccelerationBlockSize = Vector3(90, 0, 30);

	AABBVolume* volume = new AABBVolume(AccelerationBlockSize / 2);

	AccelerationBlock->SetBoundingVolume((CollisionVolume*)volume);
	AccelerationBlock->GetTransform()
		.SetScale(AccelerationBlockSize)
		.SetPosition(position);

	AccelerationBlock->SetRenderObject(new RenderObject(&AccelerationBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	AccelerationBlock->SetPhysicsObject(new PhysicsObject(&AccelerationBlock->GetTransform(), AccelerationBlock->GetBoundingVolume()));

	AccelerationBlock->GetPhysicsObject()->SetInverseMass(0);
	AccelerationBlock->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(AccelerationBlock);

	return AccelerationBlock;
}

GameObject* TutorialGame::AddAccelerationBlock3ToWorld(const Vector3& position) {
	GameObject* AccelerationBlock = new GameObject("Block3");

	Vector3 AccelerationBlockSize = Vector3(90, 0, 30);

	AABBVolume* volume = new AABBVolume(AccelerationBlockSize / 2);

	AccelerationBlock->SetBoundingVolume((CollisionVolume*)volume);
	AccelerationBlock->GetTransform()
		.SetScale(AccelerationBlockSize)
		.SetPosition(position);

	AccelerationBlock->SetRenderObject(new RenderObject(&AccelerationBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	AccelerationBlock->SetPhysicsObject(new PhysicsObject(&AccelerationBlock->GetTransform(), AccelerationBlock->GetBoundingVolume()));

	AccelerationBlock->GetPhysicsObject()->SetInverseMass(0);
	AccelerationBlock->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(AccelerationBlock);

	return AccelerationBlock;
}

GameObject* TutorialGame::AddAccelerationBlock4ToWorld(const Vector3& position) {
	GameObject* AccelerationBlock = new GameObject("Block4");

	Vector3 AccelerationBlockSize = Vector3(5, 0, 5);

	AABBVolume* volume = new AABBVolume(AccelerationBlockSize / 2);

	AccelerationBlock->SetBoundingVolume((CollisionVolume*)volume);
	AccelerationBlock->GetTransform()
		.SetScale(AccelerationBlockSize)
		.SetPosition(position);

	AccelerationBlock->SetRenderObject(new RenderObject(&AccelerationBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	AccelerationBlock->SetPhysicsObject(new PhysicsObject(&AccelerationBlock->GetTransform(), AccelerationBlock->GetBoundingVolume()));

	AccelerationBlock->GetPhysicsObject()->SetInverseMass(0);
	AccelerationBlock->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(AccelerationBlock);

	return AccelerationBlock;
}

GameObject* TutorialGame::AddAccelerationBlock5ToWorld(const Vector3& position) {
	GameObject* AccelerationBlock = new GameObject("Block5");

	Vector3 AccelerationBlockSize = Vector3(90, 0, 30);

	AABBVolume* volume = new AABBVolume(AccelerationBlockSize / 2);

	AccelerationBlock->SetBoundingVolume((CollisionVolume*)volume);
	AccelerationBlock->GetTransform()
		.SetScale(AccelerationBlockSize)
		.SetPosition(position);

	AccelerationBlock->SetRenderObject(new RenderObject(&AccelerationBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	AccelerationBlock->SetPhysicsObject(new PhysicsObject(&AccelerationBlock->GetTransform(), AccelerationBlock->GetBoundingVolume()));

	AccelerationBlock->GetPhysicsObject()->SetInverseMass(0);
	AccelerationBlock->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(AccelerationBlock);

	return AccelerationBlock;
}

GameObject* TutorialGame::AddWinPointToWorld(const Vector3& position) {
	GameObject* WinPoint = new GameObject("Win");

	Vector3 WinPointSize = Vector3(60, 0, 30);

	AABBVolume* volume = new AABBVolume(WinPointSize / 2);

	WinPoint->SetBoundingVolume((CollisionVolume*)volume);
	WinPoint->GetTransform()
		.SetScale(WinPointSize)
		.SetPosition(position);

	WinPoint->SetRenderObject(new RenderObject(&WinPoint->GetTransform(), cubeMesh, basicTex, basicShader));
	WinPoint->SetPhysicsObject(new PhysicsObject(&WinPoint->GetTransform(), WinPoint->GetBoundingVolume()));

	WinPoint->GetPhysicsObject()->SetInverseMass(0);
	WinPoint->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(WinPoint);

	return WinPoint;
}

GameObject* TutorialGame::AddCoinToWorld(const Vector3& position, string name) {
	GameObject* apple = new GameObject(name);

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(0.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

//jiajiajia
GameObject* TutorialGame::AddSlopeToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(15, 2, 15);
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);
	floor->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(15, 2, 15), 45));
	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, floorTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}



GameObject* TutorialGame::AddWallBeforeAndAfter2ToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(1, 3, 10);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, wallTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}//add wall to world
GameObject* TutorialGame::AddWallLeftAndRight2ToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(10, 3, 1);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, wallTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	//floor->GetPhysicsObject()->set

	world->AddGameObject(floor);

	return floor;
}//add wall to world

GameObject* TutorialGame::AddFloor3ToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("LoseLand");

	Vector3 floorSize = Vector3(1000, 2, 1000);
	AABBVolume* volume = new AABBVolume(floorSize);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, floorTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
//add wall to world

GameObject* TutorialGame::AddWallBeforeAndAfter3ToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("LoseLand");

	Vector3 floorSize = Vector3(1, 100, 1000);
	AABBVolume* volume = new AABBVolume(floorSize);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, wallTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}//add wall to world
GameObject* TutorialGame::AddWallLeftAndRight3ToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("LoseLand");

	Vector3 floorSize = Vector3(1000, 100, 1);
	AABBVolume* volume = new AABBVolume(floorSize);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, wallTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();
	//floor->GetPhysicsObject()->set

	world->AddGameObject(cube);

	return cube;
}//add wall to world

GameObject* TutorialGame::AddWinPoint2ToWorld(const Vector3& position) {
	GameObject* WinPoint = new GameObject("Win");

	Vector3 WinPointSize = Vector3(3, 0, 3);

	AABBVolume* volume = new AABBVolume(WinPointSize / 2);

	WinPoint->SetBoundingVolume((CollisionVolume*)volume);
	WinPoint->GetTransform()
		.SetScale(WinPointSize)
		.SetPosition(position);

	WinPoint->SetRenderObject(new RenderObject(&WinPoint->GetTransform(), cubeMesh, basicTex, basicShader));
	WinPoint->SetPhysicsObject(new PhysicsObject(&WinPoint->GetTransform(), WinPoint->GetBoundingVolume()));

	WinPoint->GetPhysicsObject()->SetInverseMass(0);
	WinPoint->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(WinPoint);

	return WinPoint;
}

GameObject* TutorialGame::AddMiddle8ToWorld(const Vector3& position) {
	GameObject* Obstacle6 = new GameObject();

	Vector3 Obstacle6Size = Vector3(1, 3, 15);

	AABBVolume* volume = new AABBVolume(Obstacle6Size / 2);

	Obstacle6->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle6->GetTransform()
		.SetScale(Obstacle6Size)
		.SetPosition(position);

	Obstacle6->SetRenderObject(new RenderObject(&Obstacle6->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle6->SetPhysicsObject(new PhysicsObject(&Obstacle6->GetTransform(), Obstacle6->GetBoundingVolume()));

	Obstacle6->GetPhysicsObject()->SetInverseMass(0);
	Obstacle6->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle6);

	return Obstacle6;
}



GameObject* TutorialGame::AddMiddle9ToWorld(const Vector3& position) {
	GameObject* Obstacle6 = new GameObject();

	Vector3 Obstacle6Size = Vector3(1, 3, 15);

	AABBVolume* volume = new AABBVolume(Obstacle6Size / 2);

	Obstacle6->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle6->GetTransform()
		.SetScale(Obstacle6Size)
		.SetPosition(position);

	Obstacle6->SetRenderObject(new RenderObject(&Obstacle6->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle6->SetPhysicsObject(new PhysicsObject(&Obstacle6->GetTransform(), Obstacle6->GetBoundingVolume()));

	Obstacle6->GetPhysicsObject()->SetInverseMass(0);
	Obstacle6->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle6);

	return Obstacle6;
}

GameObject* TutorialGame::AddMiddle10ToWorld(const Vector3& position) {
	GameObject* Obstacle6 = new GameObject();

	Vector3 Obstacle6Size = Vector3(1, 3, 15);

	AABBVolume* volume = new AABBVolume(Obstacle6Size / 2);

	Obstacle6->SetBoundingVolume((CollisionVolume*)volume);
	Obstacle6->GetTransform()
		.SetScale(Obstacle6Size)
		.SetPosition(position);

	Obstacle6->SetRenderObject(new RenderObject(&Obstacle6->GetTransform(), cubeMesh, wallTex, basicShader));
	Obstacle6->SetPhysicsObject(new PhysicsObject(&Obstacle6->GetTransform(), Obstacle6->GetBoundingVolume()));

	Obstacle6->GetPhysicsObject()->SetInverseMass(0);
	Obstacle6->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(Obstacle6);

	return Obstacle6;
}

GameObject* TutorialGame::AddFloor2ToWorld(const Vector3& position) {
	GameObject* cube = new GameObject("jumppad");

	Vector3 floorSize = Vector3(10, 2, 10);
	AABBVolume* volume = new AABBVolume(floorSize);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, floorTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}
//coursework function end