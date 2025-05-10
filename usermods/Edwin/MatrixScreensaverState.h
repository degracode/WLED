#pragma once

#include "ScreensaverState.h"

class MatrixScreensaverState : public ScreensaverState {
public:
    virtual void OnEnter() {
        ScreensaverState::OnEnter();

        _drops.clear();

        unsigned long time = millis();
        for (int i = 0; i < 8; ++i) {
            Drop drop = Drop(time - random16(150), i, -random8(15));
            _drops.push_back(drop);
        }
    }
    virtual void OnEffect() {
        Segment& seg = SEGMENT;

        unsigned long time = millis();
        unsigned long frameTime = FRAMETIME;

        seg.fill(0);
        uint8_t matrix[8 * 8];
        std::fill(std::begin(matrix), std::end(matrix), UINT8_MAX);

        for (Drop& drop : _drops) {
            unsigned long age = time - drop.startTimeMillis;
            if (age % drop.tickInterval < frameTime) {
                drop.y += 1;
            }

            auto blendPixel = [&](int x, int y, int priority, uint32_t col) {
                if (y < 0 || y > 7 || x < 0 || x > 7)
                    return;
                if (matrix[x * 8 + y] < priority)
                    return;
                matrix[x * 8 + y] = priority;
                seg.setPixelColorXY(x, y, col);
            };

            blendPixel(drop.x, drop.y, 0, RGBW32(64, 255, 64, 0));

            int steps = 8;
            for (int i = 1; i < steps; ++i) {
                blendPixel(drop.x, drop.y - i, i + 1, RGBW32(0, (255 / steps) * (steps - i), 0, 0));
            }

            // If the star is dead or off-screen, rebirth it somewhere else
            if (drop.x < 0 || drop.x > 7
             || drop.y >= 8+steps) {
                drop = Drop(time, drop.x, -random8(15));
            }
        }

        RandomTransitionIfMillisElapsed(10000);
    }

private:
    struct Drop {
        Drop(unsigned long startTimeMillis, int x, int y)
            : x(x)
            , y(y)
            , startTimeMillis(startTimeMillis)
            , tickInterval(random8(100, 150)) {
        }
            
        int x;
        int y;
        unsigned long startTimeMillis;
        int tickInterval;
    };

    std::vector<Drop> _drops;
};