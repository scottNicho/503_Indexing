#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "GameObject.h"
#include "CollisionDetection.h"
#include "Quaternion.h"

#include "Constraint.h"
#include <chrono>
#include "Debug.h"
#include "Window.h"
#include <functional>
#include<iostream>
#include"BPtree.h"
#include <fstream>
using namespace NCL;
using namespace CSC8503;

PhysicsSystem::PhysicsSystem(GameWorld& g, CollisionDMethod NewDetectionMethode) : gameWorld(g) {
	applyGravity = false;
	useBroadPhase = true;
	DetectionMethode = NewDetectionMethode;
	dTOffset = 0.0f;
	globalDamping = 0.995f;
	damping = 0.4f;
	SetGravity(Vector3(0.0f, -9.8f, 0.0f));

}

PhysicsSystem::~PhysicsSystem() {
	//added construction
	//destroy_tree(bptree);
	//bptree = nullptr;
	//end added construction
}

void PhysicsSystem::SetGravity(const Vector3& g) {
	gravity = g;
}

void NCL::CSC8503::PhysicsSystem::SetDamping(const float& d)
{
	damping = d;
}

float NCL::CSC8503::PhysicsSystem::GetDamping() const
{
	return damping;
}

/*

If the 'game' is ever reset, the PhysicsSystem must be
'cleared' to remove any old collisions that might still
be hanging around in the collision list. If your engine
is expanded to allow objects to be removed from the world,
you'll need to iterate through this collisions list to remove
any collisions they are in.

*/
void PhysicsSystem::Clear() {
	allCollisions.clear();
}

/*

This is the core of the physics engine update

*/

bool useSimpleContainer = false;

int constraintIterationCount = 10;

//This is the fixed timestep we'd LIKE to have
const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

/*
This is the fixed update we actually have...
If physics takes too long it starts to kill the framerate, it'll drop the
iteration count down until the FPS stabilises, even if that ends up
being at a low rate.
*/
int realHZ = idealHZ;
float realDT = idealDT;

void PhysicsSystem::Update(float dt) {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::B)) {
		useBroadPhase = !useBroadPhase;
		std::cout << "Setting broadphase to " << useBroadPhase << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::N)) {
		useSimpleContainer = !useSimpleContainer;
		std::cout << "Setting broad container to " << useSimpleContainer << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::I)) {
		constraintIterationCount--;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::O)) {
		constraintIterationCount++;
		std::cout << "Setting constraint iterations to " << constraintIterationCount << std::endl;
	}

	dTOffset += dt; //We accumulate time delta here - there might be remainders from previous frame!

	GameTimer t;
	t.GetTimeDeltaSeconds();
	Vector3 still = { 0,0,0 };
	if (useBroadPhase) {
		//add statment for diffrent collision structures 
		UpdateObjectAABBs();
		for (GameObject* obj : GetGameWorld().GetGameObjects()) {
			if (obj->GetPhysicsObject()->GetLinearVelocity() != still) {
				int keyNum = obj->GetKeyValue();
				//delete_(bptree,keyNum);
				InsertGameObjectIntoBTree(obj, true);
			}
		}
	}
	int iteratorCount = 0;

	std::ofstream csvFile("physics_times.csv", std::ios::app);

	if (!csvFile.is_open()) {
		std::cerr << "Failed to open CSV file for writing!" << std::endl;
		return;
	}

	while (dTOffset > realDT) {
		IntegrateAccel(realDT); //Update accelerations from external forces
		if (useBroadPhase) {

			auto startTime = std::chrono::high_resolution_clock::now();
			switch (DetectionMethode)
			{
			case(0):
				break;
			case(1):
				BroadPhaseQuadTree();
				break;
			case(2):
				BroadPhaseBppTree();
				break;
			case(3):
				BroadPhaseNotConstantBppTree();
				break;
			case(4):
				BroadPhaseConstantRBTree();
				break;
			default:
				break;
			}



			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
			//std::cout << "Execution time: " << duration << " ms" << std::endl;
			csvFile << duration << "\n";
			csvFile.close();

			auto narrowPhaseStartTime = std::chrono::high_resolution_clock::now();

			NarrowPhase();

			// Calculate narrow phase duration
			auto narrowPhaseEndTime = std::chrono::high_resolution_clock::now();
			auto narrowPhaseDuration = std::chrono::duration<double, std::milli>(narrowPhaseEndTime - narrowPhaseStartTime).count();
			std::ofstream narrowPhaseFile("narrow_phase_times.csv", std::ios_base::app);
			if (narrowPhaseFile.is_open()) {
				narrowPhaseFile << std::chrono::duration<double, std::milli>(narrowPhaseEndTime - narrowPhaseStartTime).count() << "\n";
				narrowPhaseFile.close();
			}
			else {
				std::cerr << "Unable to open narrow_phase_times.csv for writing.\n";
			}
		}
		else {
			BasicCollisionDetection();
		}

		//This is our simple iterative solver - 
		//we just run things multiple times, slowly moving things forward
		//and then rechecking that the constraints have been met		
		float constraintDt = realDT / (float)constraintIterationCount;
		for (int i = 0; i < constraintIterationCount; ++i) {
			UpdateConstraints(constraintDt);
		}
		IntegrateVelocity(realDT); //update positions from new velocity changes

		dTOffset -= realDT;
		iteratorCount++;
	}

	ClearForces();	//Once we've finished with the forces, reset them to zero

	UpdateCollisionList(); //Remove any old collisions

	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();

	//Uh oh, physics is taking too long...
	if (updateTime > realDT) {
		realHZ /= 2;
		realDT *= 2;
		std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
	}
	else if (dt * 2 < realDT) { //we have plenty of room to increase iteration count!
		int temp = realHZ;
		realHZ *= 2;
		realDT /= 2;

		if (realHZ > idealHZ) {
			realHZ = idealHZ;
			realDT = idealDT;
		}
		if (temp != realHZ) {
			std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
		}
	}
}
/*
Later on we're going to need to keep track of collisions
across multiple frames, so we store them in a set.

The first time they are added, we tell the objects they are colliding.
The frame they are to be removed, we tell them they're no longer colliding.

From this simple mechanism, we we build up gameplay interactions inside the
OnCollisionBegin / OnCollisionEnd functions (removing health when hit by a
rocket launcher, gaining a point when the player hits the gold coin, and so on).
*/
void PhysicsSystem::UpdateCollisionList() {
	for (std::set<CollisionDetection::CollisionInfo>::iterator i = allCollisions.begin(); i != allCollisions.end(); ) {
		if ((*i).framesLeft == numCollisionFrames) {
			i->a->OnCollisionBegin(i->b);
			i->b->OnCollisionBegin(i->a);
			if (i->a->bTriggerDelete || i->b->bTriggerDelete) {
				i = allCollisions.erase(i);
				continue;
			}
		}

		CollisionDetection::CollisionInfo& in = const_cast<CollisionDetection::CollisionInfo&>(*i);
		in.framesLeft--;

		if ((*i).framesLeft < 0) {
			i->a->OnCollisionEnd(i->b);
			i->b->OnCollisionEnd(i->a);
			i = allCollisions.erase(i);
		}
		else {
			++i;
		}
	}
}

void PhysicsSystem::UpdateObjectAABBs() {
	gameWorld.OperateOnContents(
		[](GameObject* g) {
			g->UpdateBroadphaseAABB();
		}
	);
}

/*

This is how we'll be doing collision detection in tutorial 4.
We step thorugh every pair of objects once (the inner for loop offset
ensures this), and determine whether they collide, and if so, add them
to the collision set for later processing. The set will guarantee that
a particular pair will only be added once, so objects colliding for
multiple frames won't flood the set with duplicates.
*/
void PhysicsSystem::BasicCollisionDetection() {


	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if ((*i)->GetPhysicsObject() == nullptr) {
			continue;

		}
		for (auto j = i + 1; j != last; ++j) {
			if ((*j)->GetPhysicsObject() == nullptr) {
				continue;

			}
			CollisionDetection::CollisionInfo info;
			if (CollisionDetection::ObjectIntersection(*i, *j, info)) {
				//std::cout << "Collision between " << (*i)->GetName() << " and " << (*j)->GetName() << std::endl;
				ImpulseResolveCollision(*info.a, *info.b, info.point);
				info.framesLeft = numCollisionFrames;
				allCollisions.insert(info);
			}

		}
	}
}

/*

In tutorial 5, we start determining the correct response to a collision,
so that objects separate back out.

*/
void PhysicsSystem::ImpulseResolveCollision(GameObject& a, GameObject& b, CollisionDetection::ContactPoint& p) const {


	PhysicsObject* physA = a.GetPhysicsObject();
	PhysicsObject* physB = b.GetPhysicsObject();

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0) {
		return; //two static objects ??
	}


	// Separate them out using projection
	transformA.SetPosition(transformA.GetPosition() - (p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));
	transformB.SetPosition(transformB.GetPosition() + (p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));

	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA =
		Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB =
		Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	//now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() *
		Vector3::Cross(relativeA, p.normal), relativeA);
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() *
		Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	float cRestitution = 0.66f * physA->GetElasticity() + physB->GetElasticity(); // disperse some kinectic energy

	float j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);
	Vector3 fullImpulse = p.normal * j;

	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);
	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));
}

/*

Later, we replace the BasicCollisionDetection method with a broadphase
and a narrowphase collision detection method. In the broad phase, we
split the world up using an acceleration structure, so that we can only
compare the collisions that we absolutely need to.

*/
void PhysicsSystem::BroadPhaseQuadTree() {

	broadphaseCollisions.clear();
	QuadTree <GameObject*> tree(Vector2(1024, 1024), 7, 6);

	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	for (auto i = first; i != last; ++i) {
		Vector3 halfSizes;
		if (!(*i)->GetBroadphaseAABB(halfSizes)) {
			continue;
		}
		Vector3 pos = (*i)->GetTransform().GetPosition();
		tree.Insert(*i, pos, halfSizes);
		tree.OperateOnContents([&](std::list <QuadTreeEntry <GameObject*>>& data) {
			CollisionDetection::CollisionInfo info;
			for (auto i = data.begin(); i != data.end(); ++i) {
				for (auto j = std::next(i); j != data.end(); ++j) {
					//is this pair of items already in the collision set -
						//if the same pair is in another quadtree node together etc
					info.a = std::min((*i).object, (*j).object);
					info.b = std::max((*i).object, (*j).object);
					broadphaseCollisions.insert(info);

				}
			}
			});
	}
}


constexpr float back_transformation_scaling_value = 50.0;

void PhysicsSystem::finalise_initialisation() {
	//addded construction for BroadPhaseInConstantBppTree
	min_z_value = std::numeric_limits<unsigned long long>::max();
	Vector3 low;
	Vector3 high;
	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	//std::unordered_map< GameObject*, std::set<unsigned long long>> points;
	for (auto i = first; i != last; ++i) {
		//std::cout << (*i)->GetName() << std::endl;
		(*i)->ClearZValues(); // ONly in this old logic! Because, we know the bptree is empty, and therefore, has no previous point
		InsertGameObjectIntoBTree(*i);
	}

	//print_tree();
	//std::cout << "";
	//print_tree(bptree, true);
	// end added construction
}

void PhysicsSystem::RedBlack_initialisation() {
	min_z_value = std::numeric_limits<unsigned long long>::max();
	Vector3 low;
	Vector3 high;
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->ClearZValues();
		InsertGameObjectIntoRBTree(*i);
	}
}

void PhysicsSystem::BroadPhaseBppTree() {
	broadphasemap(bptree2, true);
#if 0
	//QuadTree <GameObject*> tree(Vector2(1024, 1024), 7, 6);
	//print_tree(bptree);
	broadphaseCollisions.clear();
	collisions_being_checked.clear();
	finalise_initialisation();

	//print_tree();
	//std::cout << "";

#if 0
	// BEGIN: part to be moved in the constructor, but only in the most recent version of the code!
	min_z_value = std::numeric_limits<unsigned long long>::max();
	//Vector3 low;
	//Vector3 high;
	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	//std::unordered_map< GameObject*, std::set<unsigned long long>> points;
	for (auto i = first; i != last; ++i) {
		(*i)->ClearZValues(); // ONly in this old logic! Because, we know the bptree is empty, and therefore, has no previous point
		InsertGameObjectIntoBTree(*i);
	}
	// END: part to be moved...
#endif

	//exit(1);
	std::pair<GameObject*, GameObject*> cp;

	for (const auto& [k, v] : bptree2) {
		if (v.size() > 1) {
			std::vector<GameObject*> tmp{ v.begin(), v.end() };
			CollisionDetection::CollisionInfo info;
			for (int j = 0; j < v.size(); j++)
				for (int k = 0; k < j; k++) {
					info.a = cp.first = std::min(tmp.at(k), tmp.at(j));
					info.b = cp.second = std::max(tmp.at(k), tmp.at(j));
					if (collisions_being_checked.insert(cp).second)
						broadphaseCollisions.insert(info);
				}
			//std::cout << v.size() << std::endl;
			//v.clear();
		}
	}

#if 0
	/*auto itSet = points.begin();
	auto& setId1 = itSet->second;
	itSet++;
	auto& setId2 = itSet->second;
	std::vector<unsigned long long> result;
	std::set_intersection(setId1.begin(), setId1.end(), setId2.begin(), setId2.end(), std::back_inserter(result));
	*/
	std::pair<GameObject*, GameObject*> cp;
	int i;
	std::vector<GameObject*> v;
	node* n = find_leaf(bptree, min_z_value, false);// Finding the element from the tree
	if (n == nullptr)
		return; // Nothing was found
	for (i = 0; i < (n->order - 1) && (unsigned long long)n->keys[i] < (unsigned long long)min_z_value; i++) // getting the key within the node if exists
		;
	if (i == n->num_keys)
		return; // Nothing was found: beyond the set of elements


	unsigned long long CurrentZ = n->keys[i];
	//std::vector<GameObject*> LastObjects{};

	while (n != nullptr) {
		// TASK: collect in v all the game objects, n->pointers[i], having all the same z value, n->keys[i]

		/*if (std::lower_bound(result.begin(), result.end(), CurrentZ) == result.end()) {
			std::cout << "not in intersection!" << std::endl;
		} else {
			std::cout << "in intersection!" << std::endl;
		}*/

		while ((n->keys[i] == CurrentZ) && (i < n->num_keys)) {
			if (n->pointers[i])
				v.push_back(static_cast<GameObject*>(n->pointers[i]));
			i++;
		}

		auto next_leaf = static_cast<node*>(n->pointers[n->order - 1]);

		if (((i == n->num_keys) && !next_leaf) || (((i != n->num_keys)) && (n->keys[i] != CurrentZ))) {
			std::sort(v.begin(), v.end());
			v.erase(std::unique(v.begin(), v.end()), v.end());
			if (v.size() > 1) {
				// As soon as you detect a new different key, that is, as soon as you finish scanning all the nodes with the same z value,
				// Do a cross product across all the pointers associated to it
				CollisionDetection::CollisionInfo info;
				for (int j = 0; j < v.size(); j++)
					for (int k = 0; k < j; k++) {
						info.a = cp.first = std::min(v.at(i), v.at(j));
						info.b = cp.second = std::max(v.at(i), v.at(j));
						if (collisions_being_checked.insert(cp).second)
							broadphaseCollisions.insert(info);
					}
				std::cout << v.size() << std::endl;
				v.clear();
			}
		}

		/*for (; i < n->num_keys; i++) { // Iterating over the existing elements
			returned_keys[num_found] = n->keys[i];
			returned_pointers[num_found] = n->pointers[i];
			num_found++;
		}*/
		if (i == n->num_keys) {
			n = next_leaf; // Moving towards the next leaf
			i = 0; // re-setting the counter to zero
		}
		else {
			CurrentZ = n->keys[i];
		}
	}
#endif


	//bptree.clear();
	bptree2.clear();

#if 0
	// Old way: delete the tree all the times
	// New code: only to be called in the destructor
	destroy_tree(bptree);
	bptree = nullptr;
#endif
#endif 
}


void PhysicsSystem::BroadPhaseNotConstantBppTree() {
	broadphasemap(bptree2, false);
#if 0
	broadphaseCollisions.clear();
	collisions_being_checked.clear();

	// BEGIN: part to be moved in the constructor, but only in the most recent version of the code!

	// END: part to be moved...


	std::pair<GameObject*, GameObject*> cp;
	for (const auto& [k, v] : bptree2) {
		if (v.size() > 1) {
   			std::vector<GameObject*> tmp{ v.begin(), v.end() };
			CollisionDetection::CollisionInfo info;
			for (int j = 0; j < v.size(); j++)
				for (int k = j +1; k < v.size(); k++) {
					info.a = cp.first = std::min(tmp.at(j), tmp.at(k));
					info.b = cp.second = std::max(tmp.at(j), tmp.at(k));
					if (collisions_being_checked.insert(cp).second)
						broadphaseCollisions.insert(info);
				}
			//std::cout << v.size() << std::endl;
			//v.clear();
		}
	}

	// Old way: delete the tree all the times
	// New code: only to be called in the destructor
#endif
}


void PhysicsSystem::InsertGameObjectIntoBTree(GameObject* gameObject, bool check_containment) {
	Vector3 halfSizes;
	if (!(gameObject)->GetBroadphaseAABB(halfSizes)) {
		return;
	}
	//(*i)->GetBoundingVolume();
	Vector3 pos = (gameObject)->GetTransform().GetPosition();
	/*Vector3 halfSize2;
	if (!(*i)->GetBroadphaseAABB(halfSize2)) {
		continue;
	}*/
	Vector3 low = (((gameObject)->GetTransform().GetPosition()) - halfSizes) / back_transformation_scaling_value;
	Vector3 high = (((gameObject)->GetTransform().GetPosition()) + halfSizes) / back_transformation_scaling_value;

	int low_x, low_z, high_x, high_z;
	unsigned int low_fx, low_fz, high_fx, high_fz;
	low_x = (int)std::floorf(std::min(low.x, high.x));
	high_x = (int)std::ceilf(std::max(low.x, high.x));
	low_z = (int)std::floorf(std::min(low.z, high.z));
	high_z = (int)std::ceilf(std::max(low.z, high.z));

#define CD(low_x, low_fx) \
		if (low_x < 0) {\
			low_fx = (std::numeric_limits<int>::max()) + low_x;\
		}\
		else {\
			low_fx = (unsigned int)(std::numeric_limits<int>::max()) + (unsigned int)low_x;\
		}\

	CD(low_x, low_fx)
		CD(low_z, low_fz)
		CD(high_x, high_fx)
		CD(high_z, high_fz)

		auto old_z_values = (gameObject)->GetZvalue();
	std::set<unsigned long long> new_set_values;
	//std::cout << "GO: " << *i << std::endl;
	for (unsigned int x = low_fx; x <= high_fx; x++) {
		for (unsigned int z = low_fz; z <= high_fz; z++) {
			unsigned long long Z_Value = encode_morton_2d(x, z);
			if (check_containment && old_z_values.contains(Z_Value)) continue;
			if (Z_Value < min_z_value) {
				min_z_value = Z_Value;     //set  the new minimum
			}
			new_set_values.insert(Z_Value);
		}
	}

	auto old_first = old_z_values.begin();
	auto last1 = old_z_values.end();
	auto novel_first = new_set_values.begin();
	auto last2 = new_set_values.end();
	bool finished = false;
	while (old_first != last1) {
		if ((novel_first == last2) && check_containment) {
			while (old_first != last1) {
				//print_tree(bptree, true);
				auto outcome = RemoveGameObjectWithZValue(gameObject, *old_first);
				old_first++;
			}
			finished = true;
		}
		if (finished) break;
		if (*old_first > *novel_first) {
			min_z_value = std::min(min_z_value, *novel_first);
			switch (DetectionMethode) {
				case None:
				case QuadTreeY:
					break;
				case BpTree_const:
				case BpTree_NotConst:
					bptree2[*novel_first].emplace(gameObject);
					break;
				case RedBlackTree_const:
				case RedBlackTree:
					RBtree[*novel_first].emplace(gameObject);
					break;
			}
			//bptree = insert(bptree, *novel_first, (void*)gameObject);
			novel_first++;
		}
		else if (*old_first < *novel_first) {
			if (check_containment) {
				//print_tree(bptree, true);
				auto outcome = RemoveGameObjectWithZValue(gameObject, *old_first);
				old_first++;
			}
		}
		else {
			old_first++;
			novel_first++;
			// Elements at intersection
		}
	}
	while (novel_first != last2) {
		min_z_value = std::min(min_z_value, *novel_first);
		switch (DetectionMethode) {
		case None:
		case QuadTreeY:
			break;
		case BpTree_const:
		case BpTree_NotConst:
			bptree2[*novel_first].insert(gameObject);
			break;
		case RedBlackTree_const:
		case RedBlackTree:
			RBtree[*novel_first].emplace(gameObject);
			break;
		}

		//bptree = insert(bptree, *novel_first, (void*)gameObject);
		gameObject->SetKeyValue(*novel_first); //set key value
		novel_first++;
	}
	//std::copy(novel_first, last2, std::back_inserter(out));


	(gameObject)->ClearZValues();
	for (const auto z : new_set_values) gameObject->SetZvalue(z);

}


bool PhysicsSystem::RemoveGameObjectWithZValue(GameObject* gameObject, unsigned long long Z_value) {
	switch (DetectionMethode) {
		case None:
		case QuadTreeY:
			break;

		case BpTree_const:
		case BpTree_NotConst: {
			auto it = bptree2.find(Z_value);
			if (it == bptree2.end()) return false;
			else {
				return (it->second.erase(gameObject) == 1);
			}
		} break;

		case RedBlackTree_const:
		case RedBlackTree: {
			auto it = RBtree.find(Z_value);
			if (it == RBtree.end()) return false;
			else {
				return (it->second.erase(gameObject) == 1);
			}
		} break;
	}

}
/*

The broadphase will now only give us likely collisions, so we can now go through them,
and work out if they are truly colliding, and if so, add them into the main collision list
*/
void PhysicsSystem::NarrowPhase() {

	for (std::set <CollisionDetection::CollisionInfo >::iterator i = broadphaseCollisions.begin();
		i != broadphaseCollisions.end(); ++i) {
		CollisionDetection::CollisionInfo info = *i;
		/*if (info.a->GetName() == "Goaty"|| info.b->GetName() == "Goaty") {
			if (info.a->GetName() == "floor" || info.b->GetName() == "floor")
			{
				std::cout << "Goat & floor" << std::endl;
			};
		} 
		if (info.a->GetName() == "Goaty" || info.b->GetName() == "Goaty") {
			if (info.a->GetName() == "next goat" || info.b->GetName() == "next goat")
			{
				std::cout << "Goat & next goat" << std::endl;
			};
		}
		if (info.a->GetName() == "floor" || info.b->GetName() == "floor") {
			if (info.a->GetName() == "next goat" || info.b->GetName() == "next goat")
			{
				std::cout << "floor & next goat" << std::endl;
			};
		}*/
		/*if (info.a->GetName() == "next goat") {
			std::cout << info.b->GetName() << std::endl;
		}*/

		if (CollisionDetection::ObjectIntersection(info.a, info.b, info)) {
			info.framesLeft = numCollisionFrames;
			ImpulseResolveCollision(*info.a, *info.b, info.point);
			allCollisions.insert(info); // insert into our main set
		}
	}
}

void PhysicsSystem::InsertGameObjectIntoRBTree(GameObject* gameObject, bool checkContainment ) {
	InsertGameObjectIntoBTree(gameObject, checkContainment);
#if 0
	// Calculate the object's Z value for broadphase grouping
	Vector3 ObjectPosition = gameObject->GetTransform().GetPosition();
	unsigned long long Z_value = CalculateZValue(ObjectPosition.x,ObjectPosition.y);

	// Insert the object into the RBtree
	RBtree[Z_value].insert(gameObject);

	// Check for containment in other cells if needed
	if (checkContainment) {
		for ( auto& [k, v] : RBtree) {
			Vector3 AABB_Halfsize;
			if (gameObject->GetBoundingVolume()->type == VolumeType::AABB) {
				AABB_Halfsize = ((AABBVolume&)*(gameObject->GetBoundingVolume())).GetHalfDimensions();
			}
			if (k != Z_value && Contains(gameObject,AABB_Halfsize, k)) {
				v.insert(gameObject);
			}
		}
	}
#endif
}

bool PhysicsSystem::RemoveGameObjectWithZZValue(GameObject* gameObject, unsigned long long Z_value) {
	return RemoveGameObjectWithZValue(gameObject, Z_value);
#if 0
	auto it = RBtree.find(Z_value);
	if (it != RBtree.end()) {
		it->second.erase(gameObject);
		if (it->second.empty()) {
			//RBtree.erase(it);
		}
		return true;
	}
	return false;
#endif
}




void PhysicsSystem::BroadPhaseConstantRBTree() {
	broadphasemap(RBtree, true);

#if 0
	//QuadTree <GameObject*> tree(Vector2(1024, 1024), 7, 6);
	//print_tree(bptree);
	broadphaseCollisions.clear();
	collisions_being_checked.clear();
	finalise_initialisation();

	//print_tree();
	//std::cout << "";

#if 0
	// BEGIN: part to be moved in the constructor, but only in the most recent version of the code!
	min_z_value = std::numeric_limits<unsigned long long>::max();
	//Vector3 low;
	//Vector3 high;
	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	//std::unordered_map< GameObject*, std::set<unsigned long long>> points;
	for (auto i = first; i != last; ++i) {
		(*i)->ClearZValues(); // ONly in this old logic! Because, we know the bptree is empty, and therefore, has no previous point
		InsertGameObjectIntoBTree(*i);
	}
	// END: part to be moved...
#endif

	//exit(1);
	std::pair<GameObject*, GameObject*> cp;

	for (const auto& [k, v] : RBtree) {
		if (v.size() > 1) {
			std::vector<GameObject*> tmp{ v.begin(), v.end() };
			CollisionDetection::CollisionInfo info;
			for (int j = 0; j < v.size(); j++)
				for (int k = 0; k < j; k++) {
					info.a = cp.first = std::min(tmp.at(k), tmp.at(j));
					info.b = cp.second = std::max(tmp.at(k), tmp.at(j));
					if (collisions_being_checked.insert(cp).second)
						broadphaseCollisions.insert(info);
				}
			//std::cout << v.size() << std::endl;
			//v.clear();
		}
	}

#if 0
	/*auto itSet = points.begin();
	auto& setId1 = itSet->second;
	itSet++;
	auto& setId2 = itSet->second;
	std::vector<unsigned long long> result;
	std::set_intersection(setId1.begin(), setId1.end(), setId2.begin(), setId2.end(), std::back_inserter(result));
	*/
	std::pair<GameObject*, GameObject*> cp;
	int i;
	std::vector<GameObject*> v;
	node* n = find_leaf(bptree, min_z_value, false);// Finding the element from the tree
	if (n == nullptr)
		return; // Nothing was found
	for (i = 0; i < (n->order - 1) && (unsigned long long)n->keys[i] < (unsigned long long)min_z_value; i++) // getting the key within the node if exists
		;
	if (i == n->num_keys)
		return; // Nothing was found: beyond the set of elements


	unsigned long long CurrentZ = n->keys[i];
	//std::vector<GameObject*> LastObjects{};

	while (n != nullptr) {
		// TASK: collect in v all the game objects, n->pointers[i], having all the same z value, n->keys[i]

		/*if (std::lower_bound(result.begin(), result.end(), CurrentZ) == result.end()) {
			std::cout << "not in intersection!" << std::endl;
		} else {
			std::cout << "in intersection!" << std::endl;
		}*/

		while ((n->keys[i] == CurrentZ) && (i < n->num_keys)) {
			if (n->pointers[i])
				v.push_back(static_cast<GameObject*>(n->pointers[i]));
			i++;
		}

		auto next_leaf = static_cast<node*>(n->pointers[n->order - 1]);

		if (((i == n->num_keys) && !next_leaf) || (((i != n->num_keys)) && (n->keys[i] != CurrentZ))) {
			std::sort(v.begin(), v.end());
			v.erase(std::unique(v.begin(), v.end()), v.end());
			if (v.size() > 1) {
				// As soon as you detect a new different key, that is, as soon as you finish scanning all the nodes with the same z value,
				// Do a cross product across all the pointers associated to it
				CollisionDetection::CollisionInfo info;
				for (int j = 0; j < v.size(); j++)
					for (int k = 0; k < j; k++) {
						info.a = cp.first = std::min(v.at(i), v.at(j));
						info.b = cp.second = std::max(v.at(i), v.at(j));
						if (collisions_being_checked.insert(cp).second)
							broadphaseCollisions.insert(info);
					}
				std::cout << v.size() << std::endl;
				v.clear();
			}
		}

		/*for (; i < n->num_keys; i++) { // Iterating over the existing elements
			returned_keys[num_found] = n->keys[i];
			returned_pointers[num_found] = n->pointers[i];
			num_found++;
		}*/
		if (i == n->num_keys) {
			n = next_leaf; // Moving towards the next leaf
			i = 0; // re-setting the counter to zero
		}
		else {
			CurrentZ = n->keys[i];
		}
	}
#endif


	//bptree.clear();
	RBtree.clear();

#if 0
	// Old way: delete the tree all the times
	// New code: only to be called in the destructor
	destroy_tree(bptree);
	bptree = nullptr;
#endif

#if 0
	broadphaseCollisions.clear();
	collisions_being_checked.clear();

	Vector3 nullVec = {0,0,0};
	std::pair<GameObject*, GameObject*> cp;
	auto ItEntry = RBtree.begin();
	if (ItEntry != RBtree.end()) {
		auto floor = *(((*ItEntry).second).begin());
		Vector3 floorHalfSize = (((AABBVolume&)*(floor->GetBoundingVolume())).GetHalfDimensions());
		++ItEntry; // Move to the second term

		for (; ItEntry != RBtree.end(); ++ItEntry) {
			auto nextObject = *(((*ItEntry).second).begin());
			auto nextObjectZValue = (((*ItEntry).first));
			// Check collision between floor and nextObject
			if (Contains(floor, floorHalfSize,nextObjectZValue)) {
				// Add the collision to broadphaseCollisions
				CollisionDetection::CollisionInfo info;
				info.a = floor;
				info.b = nextObject;
				broadphaseCollisions.insert(info);
			};
		}
	}

	for (auto it = RBtree.begin(); it != RBtree.end(); ++it) {
		const auto& [k, v] = *it;

		if (!v.empty()) {
			std::vector<GameObject*> tmp{ v.begin(), v.end() };
			CollisionDetection::CollisionInfo info;

			auto nextIt = it;
			++nextIt;

			while (nextIt != RBtree.end()) {
				const auto& [nextK, nextV] = *nextIt;

				if (!nextV.empty()) {
					for (GameObject* obj1 : v) {
						for (GameObject* obj2 : nextV) {
							info.a = cp.first = std::min(obj1, obj2);
							info.b = cp.second = std::max(obj1, obj2);

							/*if ((info.a)->GetName() == "cube" && (info.a)->GetName() == "cube") {
								continue;
							}*/
							// Check containment before adding collision
							if (Contains(info.a, ((AABBVolume&)*(info.a->GetBoundingVolume())).GetHalfDimensions(), nextK)) {
								if (collisions_being_checked.insert(cp).second) {
									broadphaseCollisions.insert(info);
								}
							}
						}
					}
				}

				++nextIt;
			}
		}
	}
#endif
#endif
}


bool PhysicsSystem::Contains(GameObject* object,Vector3 halfDims ,unsigned long long Z_value) {
	// Decode the Z_value to get x and y coordinates of the cell
	auto cellCoords = decode_morton_2d(Z_value);
	int cellX = cellCoords.first;
	int cellY = cellCoords.second;

	// Calculate the maximum vertex of the AABB
	Vector3 maxVertex = object->GetAABBMax(halfDims);
	Vector3 MinVertex = object->GetAABBMin(halfDims);
	// Check if the AABB contains any part of the cell
	// The AABB contains the cell if any of its corners are within the cell boundaries
	if (MinVertex.x <= cellX && MinVertex.y <= cellY &&
		maxVertex.x >= cellX && (maxVertex.y + 400) >= cellY) {
		return true;
	}

	return false;
}

/*
Integration of acceleration and velocity is split up, so that we can
move objects multiple times during the course of a PhysicsUpdate,
without worrying about repeated forces accumulating etc.

This function will update both linear and angular acceleration,
based on any forces that have been accumulated in the objects during
the course of the previous game frame.
*/
void PhysicsSystem::IntegrateAccel(float dt) {

	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();

		if (object == nullptr) {
			continue; //No physics object for this GameObject!
		}
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (applyGravity && inverseMass > 0) {
			accel += gravity; //dont move infinitely heavy things
		}

		linearVel += accel * dt; // integrate accel!
		object->SetLinearVelocity(linearVel);

		// Angular stuff
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor(); // update tensor vs orientation

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * dt; // integrate angular accel!
		object->SetAngularVelocity(angVel);
	}

}

/*
This function integrates linear and angular velocity into
position and orientation. It may be called multiple times
throughout a physics update, to slowly move the objects through
the world, looking for collisions.
*/
void PhysicsSystem::IntegrateVelocity(float dt) {

	std::vector <GameObject*>::const_iterator first;
	std::vector <GameObject*>::const_iterator last;
	gameWorld.GetObjectIterators(first, last);
	float frameLinearDamping = 1.0f - (damping * dt);

	for (auto i = first; i != last; ++i) {
		PhysicsObject* object = (*i)->GetPhysicsObject();
		if (object == nullptr) {
			continue;
		}
		Transform& transform = (*i)->GetTransform();
		// Position Stuff
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();
		position += linearVel * dt;
		transform.SetPosition(position);
		// Linear Damping
		linearVel = linearVel * frameLinearDamping;
		object->SetLinearVelocity(linearVel);

		// Orientation Stuff
		Quaternion orientation = transform.GetOrientation();
		Vector3 angVel = object->GetAngularVelocity();

		orientation = orientation +
			(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
		orientation.Normalise();

		transform.SetOrientation(orientation);

		//Damp the angular velocity too
		float frameAngularDamping = 1.0f - (damping * dt);
		angVel = angVel * frameAngularDamping;
		object->SetAngularVelocity(angVel);
	}
}

/*
Once we're finished with a physics update, we have to
clear out any accumulated forces, ready to receive new
ones in the next 'game' frame.
*/
void PhysicsSystem::ClearForces()
{
	gameWorld.OperateOnContents(
		[](GameObject* o)
		{
			o->GetPhysicsObject()->ClearForces();
		});
}


/*

As part of the final physics tutorials, we add in the ability
to constrain objects based on some extra calculation, allowing
us to model springs and ropes etc.

*/
void PhysicsSystem::UpdateConstraints(float dt) {
	std::vector<Constraint*>::const_iterator first;
	std::vector<Constraint*>::const_iterator last;
	gameWorld.GetConstraintIterators(first, last);

	for (auto i = first; i != last; ++i) {
		(*i)->UpdateConstraint(dt);
	}
}