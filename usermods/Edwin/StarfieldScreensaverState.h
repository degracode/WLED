#pragma once

#include "ScreensaverState.h"

class StarfieldScreensaverState : public ScreensaverState {
public:
    virtual void OnEnter() {
        ScreensaverState::OnEnter();

        _stars.clear();

        unsigned long time = millis();
        for (int i = 0; i < 5; ++i) {
            Star star = Star(time - random16(_starLifeMillis));
            _stars.push_back(star);
        }
    }
    virtual void OnEffect() {
        Segment& seg = SEGMENT;

        unsigned long time = millis();
        unsigned long frameTime = FRAMETIME;

        seg.fill(0);
        for (Star& star : _stars) {
            unsigned long age = time - star.startTimeMillis;
            if (age % 150 < frameTime) {
                star.x += star.dx;
                star.y += star.dy;
            }

            int brightness = star.brightness;
            seg.setPixelColorXY(star.x, star.y, RGBW32(brightness, brightness, brightness, 0));

            // If the star is dead or off-screen, rebirth it somewhere else
            if (age >= _starLifeMillis
            || star.x < 0 || star.x >= 8
            || star.y < 0 || star.y >= 8) {
                star = Star(time);
            }
        }

        RandomTransitionIfMillisElapsed(10000);
    }

private:
    struct Star {
        Star(unsigned long startTimeMillis = millis())
            : x(random8(2, 6))
            , y(random8(2, 6))
            , brightness(random8(64, 255))
            , startTimeMillis(startTimeMillis) {
            dx = x < 4 ? -1 : 1;
            dy = y < 4 ? -1 : 1;
        }
            
        int x;
        int y;
        int dx;
        int dy;
        int brightness;
        unsigned long startTimeMillis;
    };

    std::vector<Star> _stars;
    int _starLifeMillis = 1000;
};