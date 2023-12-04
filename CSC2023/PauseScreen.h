#pragma once
#include "PushdownState.h"
#include "Window.h"
#include <iostream>
using namespace NCL;
using namespace CSC8503;

class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(/*KeyboardKeys*/KeyCodes::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override {
		std::cout << " Press U to unpause game !\ n ";
	}
};