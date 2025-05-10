#pragma once

#include "SimpleImageState.h"

class BSODState : public SimpleImageState {
public:
    using SimpleImageState::SimpleImageState;

    virtual void OnExit() override {
        SimpleImageState::OnExit();

        _sm->ResetAllStates();
    }
};