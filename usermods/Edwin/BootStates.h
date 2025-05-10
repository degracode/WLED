#pragma once

#include "StateMachine.h"

class BIOSState : public BaseState {
public:
    virtual void OnEffect() override {
        Segment& seg = SEGMENT;
        seg.fill(0);

        auto blitHeader = [&]() {
            img_BIOS_1.blitRegion(seg, 0, 0, 6, 3);
            
        };
        auto blitHeader2 = [&]() {
            img_BIOS_1.blitRegion(seg, 6, 0, 2, 3);
            img_BIOS_1.blitRegion(seg, 0, 7, 8, 1); // footer
        };

        unsigned long time = GetStateActiveMillis();
        if (time < 500) {
            // black screen
        }
        else if (time < 2500) {
            blitHeader();
            blitHeader2();
        }
        else if (time < 9000) {
            blitHeader();
            
            if (time > 3000)
                img_BIOS_1.blitRegion(seg, 0, 4, 3, 1); // Check 1
            if (time > 4000) {
                img_BIOS_1.blitRegion(seg, 4, 4, 4, 1); // Result 1
                img_BIOS_1.blitRegion(seg, 0, 5, 3, 1); // Check 2
            }
            if (time > 5000)
                img_BIOS_1.blitRegion(seg, 4, 5, 4, 1); // Result 1

            if (time < 8000)
                blitHeader2();
        }
        else if (time < 9250) {}
        else if (time < 10000)
            img_BIOS_2.blit(seg, 0, 0);
        else if (time < 11500)
            img_BIOS_3.blit(seg, 0, 0);
        else if (time >= 12500) {
            RandomTransition();
        }
    }
};

class OSBootState : public SimpleImageState {
public:
    using SimpleImageState::SimpleImageState;

    virtual void OnEffect() override {
        SimpleImageState::OnEffect();

        _image->blitRegion(SEGMENT, 0, 7, 8, 1, 0, 7, 8, 1, -GetStateActiveMillis() / 100, 0);
    }
};