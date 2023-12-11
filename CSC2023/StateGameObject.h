#pragma once
#include "GameObject.h"

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class StateGameObject : public GameObject  {
        public:
            StateGameObject();
            ~StateGameObject();

            virtual void Update(float dt);
            void SetVert(bool v) {
                vert = v;
            }

        protected:
            void MoveLeft(float dt);
            void MoveRight(float dt);
            void MoveUp(float dt);
            void MoveDown(float dt);

            StateMachine* stateMachine;
            float counter;
            bool vert = false;
        };
    }
}
