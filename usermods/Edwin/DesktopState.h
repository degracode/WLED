#pragma once

#include "StateMachine.h"
#include "OSUtils.h"

class DesktopState : public BaseState {
public:
    virtual void Reset() override {
        BaseState::Reset();
        
        _loaded = false;

        static const std::array<const Image*, 2> wallpapers = {&img_GreenHill, &img_Clouds};
        img_Wallpaper = wallpapers[random8(wallpapers.size())];
    }

    virtual void OnEffect() override {
        DrawDesktopWallpaper();

        if (_loaded || GetStateActiveMillis() > 500) {
            DrawTaskbar(0);
        }
        if (_loaded || GetStateActiveMillis() > 1500) {
            DrawDesktopIcons();
            _loaded = true;
        }

        RandomTransitionIfMillisElapsed(5000);
    }

private:
    bool _loaded = false;
};