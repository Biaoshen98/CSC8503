#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/StateGameObject.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"


namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			//coursework function begin
			bool isSelected = false;
			void DrawMenu();
			void DrawWin();
			void DrawLose(std::string winner);
			void DrawPause();
			void InitGameWorld1();//ball
			void InitGameWorld2();//maze

			bool UpdatePushdown(float dt) {
				if (!machine->Update(dt)) {
					return false;
				}
				return true;
			}
			//coursework function end

		protected:
			//coursework begin
			PushdownMachine* machine;
			std::string winnerName;
			GameObject* player = nullptr;
			bool multiplayer = 0;
			bool pathfound;
			//coursework end
			// 
			GameObject* AddSpherePlayerToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddSpherePlayer1ToWorld(const Vector3& position, float radius, float inverseMass);
			//GameObject* AddSpherePlayer2ToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			//state machine begin
			StateGameObject* AddStateWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 0.0f);
			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject = nullptr;
			StateGameObject* testStateObject1 = nullptr;//
			StateGameObject* testStateWall1 = nullptr;//
			//state machine end

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();
			void ImpulseResolveCollision(GameObject& a, GameObject& b) const;
			void InitGameExamples();
			void InitAccelerationBlockWorld();
			void ExWallAndFloorforGame1();
			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
			//void Bridge();//bridge
			GameObject* AddJumpPad(const Vector3& position, Vector3 dimensions, float inverseMass);//jump
			//GameObject* AddIcePad(const Vector3& position, Vector3 dimensions, float inverseMass);//speed up

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddWallBeforeAndAfterToWorld(const Vector3& position);//add wall to world
			GameObject* AddWallLeftAndRightToWorld(const Vector3& position);
			GameObject* AddWallBeforeAndAfter2ToWorld(const Vector3& position);
			GameObject* AddWallLeftAndRight2ToWorld(const Vector3& position);
			//add wall to world
			GameObject* AddMiddle1ToWorld(const Vector3& position);
			GameObject* AddMiddle2ToWorld(const Vector3& position);
			GameObject* AddMiddle3ToWorld(const Vector3& position);
			GameObject* AddMiddle4ToWorld(const Vector3& position);
			GameObject* AddMiddle5ToWorld(const Vector3& position);
			GameObject* AddMiddle6ToWorld(const Vector3& position);
			GameObject* AddStartToWorld(const Vector3& position);
			GameObject* AddSpinObstacleToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, string game, float inverseMass);
			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);
			GameObject* AddAccelerationBlock1ToWorld(const Vector3& position);
			GameObject* AddAccelerationBlock2ToWorld(const Vector3& position);
			GameObject* AddAccelerationBlock3ToWorld(const Vector3& position);
			GameObject* AddAccelerationBlock4ToWorld(const Vector3& position);
			GameObject* AddAccelerationBlock5ToWorld(const Vector3& position);
			GameObject* AddWinPointToWorld(const Vector3& position);
			GameObject* AddCoinToWorld(const Vector3& position, string name);
			GameObject* AddSlopeToWorld(const Vector3& position);
			GameObject* AddFloor3ToWorld(const Vector3& position);
			GameObject* AddWallBeforeAndAfter3ToWorld(const Vector3& position);
			GameObject* AddWallLeftAndRight3ToWorld(const Vector3& position);
			GameObject* AddWinPoint2ToWorld(const Vector3& position);
			GameObject* AddMiddle8ToWorld(const Vector3& position);
			GameObject* AddMiddle9ToWorld(const Vector3& position);
			GameObject* AddMiddle10ToWorld(const Vector3& position);
			GameObject* AddFloor2ToWorld(const Vector3& position);
			void InitMap();
			void TestPathfinding();
			void DisplayPathfinding();
			GameTechRenderer* renderer;
			PhysicsSystem* physics;
			GameWorld* world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;


			OGLMesh* capsuleMesh = nullptr;
			OGLMesh* cubeMesh = nullptr;
			OGLMesh* sphereMesh = nullptr;
			OGLTexture* basicTex = nullptr;
			OGLShader* basicShader = nullptr;
			OGLTexture* floorTex = nullptr;
			OGLTexture* wallTex = nullptr;
			OGLTexture* smallwallTex = nullptr;

			//Coursework Meshes
			OGLMesh* charMeshA = nullptr;
			OGLMesh* charMeshB = nullptr;
			OGLMesh* enemyMesh = nullptr;
			OGLMesh* bonusMesh = nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			//};//tutorialgame class 
			class IntroScreen :public PushdownState {//for intro menu screen 
			protected:
				TutorialGame* sceengame;
				bool GameMode = 0;
			public:
				IntroScreen(TutorialGame* sceengame) {
					this->sceengame = sceengame;
				}
				PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
					//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
					//	*newstate = new GameScreen(sceengame, 0);
					//	return PushdownResult::Push;
					//}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM1)) {
						*newstate = new GameScreen(sceengame, 0);
						GameMode = 0;
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2)) {
						// game mode 2
						*newstate = new GameScreen(sceengame, 1);
						GameMode = 1;
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
						return PushdownResult::Pop;
					}

					sceengame->DrawMenu();
					//sceengame->UpdateGame(0);

					return PushdownState::NoChange;
				}
				void OnAwake() override {
					std::cout << "Press 1 to play single player or 2 for multiplayer. ESC to quit\n";
				}

				void OnSleep() override {
					if (GameMode == 0) {//mode 1 game ball
						std::cout << "game mode 1 here " << std::endl;
						sceengame->InitGameWorld1();
					}
					if (GameMode == 1) {//mode 2 maze ball
						std::cout << "game mode 2 here " << std::endl;
						sceengame->InitGameWorld2();//just call gamemode2 function in here
					}
				}
			};

			class GameScreen :public PushdownState {
			protected:
				TutorialGame* sceengame;
				float pausesave = 1;
				bool multiplayer;
			public:
				GameScreen(TutorialGame* sceengame, bool multiplayer = 0) {
					sceengame->multiplayer = multiplayer;
					this->sceengame = sceengame;
				}
				PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
					
					if (pausesave < 0) {
						if (Window::GetKeyboard()->KeyDown(KeyboardKeys::M)) {
							return PushdownResult::Pop;
						}
						if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
							*newstate = new PauseScreen(sceengame);
							return PushdownResult::Push;
						}
					}
					else {
						pausesave -= dt;
					}
					sceengame->UpdateGame(dt);

					return PushdownResult::NoChange;
				}
				void OnAwake() override {
					std::cout << "Resuming Game\n";
					pausesave = 0.2;
				}
			};
			class PauseScreen :public PushdownState {//for player press pause buttom screen 
			protected:
				TutorialGame* sceengame;
				float pausesave = 1;
			public:
				PauseScreen(TutorialGame* sceengame) {
					this->sceengame = sceengame;
				}
				PushdownResult OnUpdate(float dt, PushdownState** newstate) override {
					if (pausesave < 0) {
						if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
							return PushdownResult::Pop;
						}
					}
					else {
						pausesave -= dt;
					}
					sceengame->UpdateGame(0);
					//sceengame->DrawPause();
					return PushdownResult::NoChange;
				}
				void OnAwake() override {
					std::cout << "press esc to pause game" << std::endl;
					pausesave = 0.2;
				}
			};
		};//tutorialgame class end
	}
}

