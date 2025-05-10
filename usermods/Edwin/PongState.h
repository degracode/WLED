#pragma once

#include <array>
#include <vector>
#include <algorithm>

#include "StateMachine.h"

class PongState : public BaseState {
public:
    virtual void Reset() override {
        _paddles.clear();
        _paddles.push_back({0, 3, 2});
        _paddles.push_back({7, 3, 2});

        RespawnBall();
    }

    virtual void OnEffect() override {
        Segment& segment = SEGMENT;
        segment.fill(0);

        if (Every(150)) {
            if (_ball.x == 0 || _ball.x==7) {
                RespawnBall(); // win
            }

            _ball.prevX = _ball.x;
            _ball.prevY = _ball.y;
            _ball.x += _ball.dirX;
            _ball.y += _ball.dirY;
            if ((_ball.dirY==-1 && _ball.y==0) || (_ball.dirY==1 && _ball.y==7)) {
                _ball.dirY *= -1;
            }

            for (Paddle& paddle : _paddles) {
                if ((paddle.x < _ball.x && _ball.dirX < 0) || (paddle.x > _ball.x && _ball.dirX > 0)) {
                    int checkY = random8(13) == 0 ? _ball.prevY : _ball.y;
                    checkY += _ball.dirY;
                    if (paddle.y + paddle.height <= checkY) {
                        ++paddle.y;
                    } else if (paddle.y > checkY) {
                        --paddle.y;
                    }
                }
            }

            for (Paddle& paddle : _paddles) {
                if ((paddle.x < _ball.x && _ball.dirX < 0) || (paddle.x > _ball.x && _ball.dirX > 0)) {
                    if (paddle.x==_ball.x + _ball.dirX) {
                        int checkY = _ball.y + _ball.dirY;
                        if (paddle.y <= checkY && paddle.y + paddle.height > checkY) {
                            _ball.dirX *= -1;
                        }
                    }
                    break;
                }
            }
        }

        for (Paddle& paddle : _paddles) {
            for (int i = 0; i < paddle.height; ++i) {
                int y = paddle.y + i;
                if (y < 0 || y > 7)
                    continue;
                segment.setPixelColorXY(paddle.x, paddle.y + i, RGBW32(64, 64, 64, 0));
            }
        }

        segment.setPixelColorXY(_ball.x, _ball.y, RGBW32(255, 255, 255, 0));

        RandomTransitionIfMillisElapsed(10000);
    }

private:
    struct Paddle {
        int x;
        int y;
        int height;
    };

    struct Ball {
        int x;
        int y;
        int dirX;
        int dirY;
        int prevX;
        int prevY;
    };

    void RespawnBall() {
        int dirX = random8(2) ? 1 : -1;
        int x = dirX==-1 ? 6 : 1;
        int y = random8(1,6);
        _ball = {x, y, dirX, random8(2) ? 1 : -1, x, y};
    }

    std::vector<Paddle> _paddles;
    Ball _ball;
};