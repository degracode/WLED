#pragma once
#include "wled.h"

class StateMachine;

class EdwinUsermod : public Usermod {
public:
    virtual ~EdwinUsermod() {}

    virtual void setup();
    virtual void loop() {}

    uint16_t getId() { return USERMOD_ID_EDWIN; }

private:
    static uint16_t staticEffect();
    uint16_t effect();

    static EdwinUsermod* _instance;
    StateMachine* _sm = nullptr;
};