#pragma once
#include "GameScreen.h"

class IntroScreen : public PushdownState {
    PushdownResult OnUpdate(float dt, PushdownState** newState) override {
        if (Window::GetKeyboard()->KeyPressed(/*KeyboardKeys*/KeyCodes::SPACE)) {
            *newState = new GameScreen();
            return PushdownResult::Push;
        }
        if (Window::GetKeyboard()->KeyPressed(/*KeyboardKeys*/KeyCodes::ESCAPE)) {
            return PushdownResult::Pop;
        }
        return PushdownResult::NoChange;
    };

    void OnAwake() override {
        std::cout << "Welcome to a really awesome game!\n";
        std::cout << "Press Space To Begin or escape to quit!\n";
    }

    int GetID() override{
        return 0;
    }
};
