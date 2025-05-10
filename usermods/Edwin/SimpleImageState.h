#pragma once

#include "StateMachine.h"
#include "Image.h"

class SimpleImageState : public BaseState {
public:
    SimpleImageState(const Image& image, int maxActiveMillis)
    : _image(&image)
    , _maxActiveMillis(maxActiveMillis)
    {}

    virtual ~SimpleImageState(){}

    virtual void OnEffect() override {
        _image->blit(SEGMENT, 0, 0);
        RandomTransitionIfMillisElapsed(_maxActiveMillis);
    }

protected:
    const Image* _image;
    
private:
    const int _maxActiveMillis;
};