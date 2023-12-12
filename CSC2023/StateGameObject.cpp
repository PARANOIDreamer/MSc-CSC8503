#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt) -> void {
		if (vert)
			this->MoveUp(dt);
		else
			this->MoveLeft(dt);
		});

	State* stateB = new State([&](float dt) -> void {
		if (vert)
			this->MoveDown(dt);
		else
			this->MoveRight(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]() -> bool {
		return this->counter > 3.0f;
		}));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]() -> bool {
		return this->counter < 0.0f;
		}));
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -50,0,0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 50,0,0 });
	counter -= dt;
}

void StateGameObject::MoveUp(float dt) {
	GetPhysicsObject()->AddForce({ 0,0,-50 });
	counter += dt;
}

void StateGameObject::MoveDown(float dt) {
	GetPhysicsObject()->AddForce({ 0,0,50 });
	counter -= dt;
}