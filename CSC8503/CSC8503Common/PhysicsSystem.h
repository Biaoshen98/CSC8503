#pragma once
#include "../CSC8503Common/GameWorld.h"
#include <set>

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem	{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}

			void SetGravity(const Vector3& g);
		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void EmenyObject(GameObject& a, GameObject& b, char n, CollisionDetection::ContactPoint& p) const;

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();
			void jumppad(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;
			void AccelerationBlock1Object(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void AccelerationBlock2Object(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void AccelerationBlock3Object(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void AccelerationBlock4Object(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void AccelerationBlock5Object(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void Mark();
			void CoinObject(GameObject& a, GameObject& b,char n, CollisionDetection::ContactPoint& p) const;
			void SpinObstacleObject(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;
			void CheckWin(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const;

			GameWorld& gameWorld;
			
			//int TotalMark;
			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;//tutorial spatial acceleration structures

			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;
		};
	}
}

