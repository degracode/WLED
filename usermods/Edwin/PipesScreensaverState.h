#pragma once

#include <array>
#include <vector>
#include "wled.h"

#include "ScreensaverState.h"

class PipesScreensaverState : public ScreensaverState {
public:
    virtual void OnEnter() override {
        ScreensaverState::OnEnter();

        ClearCanvas();

        _numPipes = 0;

        NewPipe();
    }

    virtual void OnEffect() override {
        if (Every(150)) {
            _canvas[_x][_y] = GetColours()[_colourNum];

            _x += _dx;
            _y += _dy;

            --_lengthRemaining;
            if (_lengthRemaining==0)
                NewPipe();
            else {
                --_stepsToDirChange;
                if (_stepsToDirChange==0)
                    ChangePipeDir();
            }
        }

        Segment& segment = SEGMENT;
        for (int x = 0; x < 8; ++x)
            for (int y = 0; y < 8; ++y)
                segment.setPixelColorXY(x, y, _canvas[x][y]);

        RandomTransitionIfMillisElapsed(10000);
    }

private:
    static const std::vector<uint32_t>& GetColours() {
        static const std::vector<uint32_t> colours = {
            RGBW32(255, 0, 0, 0),
            RGBW32(0, 255, 0, 0),
            RGBW32(0, 0, 255, 0),
            RGBW32(255, 255, 0, 0),
            RGBW32(0, 255, 255, 0),
            RGBW32(255, 0, 255, 0),
            RGBW32(128, 0, 0, 0),
            RGBW32(0, 128, 0, 0),
            RGBW32(0, 0, 128, 0),
            RGBW32(128, 128, 0, 0),
            RGBW32(0, 128, 128, 0),
            RGBW32(128, 0, 128, 0),
            };
        return colours;
    }

    void ClearCanvas() {
        std::array<uint32_t, 8> zeroArray;
        zeroArray.fill(0);
        _canvas.fill(zeroArray);
    }

    void NewPipe() {
        if (_numPipes==4) {
            ClearCanvas();
            _numPipes = 0;
        }

        do
        {
            int edge = random8(4);
            int edgeStart = random8(1, 7); // Don't allow starts in the corners because it's a bit rubbish when a pipe sweeps along an edge

            switch(edge) {
            case 0:
                _x = 0;
                _y = edgeStart;
                _dx = 1;
                _dy = 0;
                break;
            case 1:
                _x = 7;
                _y = edgeStart;
                _dx = -1;
                _dy = 0;
                break;
            case 2:
                _x = edgeStart;
                _y = 0;
                _dx = 0;
                _dy = 1;
                break;
            case 3:
                _x = edgeStart;
                _y = 7;
                _dx = 0;
                _dy = -1;
                break;
            }
        } while(_canvas[_x][_y]!=0);

        _stepsToDirChange = random8(3) + 3;

        int previousColour = _colourNum;
        do {
            _colourNum = random8(GetColours().size());
        } while(_colourNum==previousColour);

        _lengthRemaining = 12;

        ++_numPipes;
    }

    void ChangePipeDir() {
        std::swap(_dx, _dy);
        int dirSwap = random8(2) == 0 ? 1 : -1;
        _dx *= dirSwap;
        _dy *= dirSwap;

        int distToEdge = 0;
        if (_dx > 0)
            distToEdge = 7 - _x;
        else if (_dx < 0)
            distToEdge = _x;
        else if (_dy > 0)
            distToEdge = 7 - _y;
        else if (_dy < 0)
            distToEdge = _y;

        if (distToEdge >= 2)
            _stepsToDirChange = random8(2, distToEdge + 1);
        else
            NewPipe();
    }

    int _x;
    int _y;
    int _dx;
    int _dy;
    int _stepsToDirChange;
    int _colourNum;
    int _lengthRemaining;
    int _numPipes;

    std::array<std::array<uint32_t, 8>, 8>  _canvas;
};
