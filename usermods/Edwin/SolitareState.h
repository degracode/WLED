#pragma once

#include <array>
#include <vector>
#include <algorithm>

#include "StateMachine.h"

class SolitareState : public BaseState {
public:
    virtual void Reset() override {
        const int deckSize = 20;

        _board.resize(4);
        _sorted.resize(4);

        _revealed = Pile();
        _revealed._num = 0;
        _revealed._max = 3;
        _revealed._faceUp = true;
        _revealed._x = 1;
        _revealed._y = 0;

        for (int pileNum = 0; pileNum < _board.size(); ++pileNum) {
            if (!_board[pileNum])
                _board[pileNum] = new Pile();
            Pile& pile = *_board[pileNum];
            pile = Pile();
            
            pile._num = pileNum + 1;
            pile._max = 5;
            pile._multiMove = true;
            pile._drawStaggered = true;
            pile._acceptMultiMove = true;
            pile._faceUp = true;
            pile._x = pileNum * 2;
            pile._y = 2;
        }

        for (int pileNum = 0; pileNum < _sorted.size(); ++pileNum) {
            if (!_sorted[pileNum])
                _sorted[pileNum] = new Pile();
            Pile& pile = *_sorted[pileNum];
            pile = Pile();

            pile._num = 0;
            pile._max = deckSize / _sorted.size();
            pile._faceUp = true;
            pile._x = 4 + pileNum;
            pile._y = 0;
        }

        _deck = Pile();
        _deck._faceUp = false;
        _deck._x = 0;
        _deck._y = 0;

        _allPiles.clear();
        _allPiles.push_back(&_deck);
        _allPiles.push_back(&_revealed);
        for (Pile* pile : _board)
            _allPiles.push_back(pile);
        for (Pile* pile : _sorted)
            _allPiles.push_back(pile);

        int cardsOut = std::accumulate(_allPiles.begin(), _allPiles.end(), 0, [](int total, const Pile* b) { return total+b->_num;});
        _deck._num = deckSize - cardsOut;
        _deck._max = deckSize;

        _stage = Stage::Deal;

        std::array<uint32_t, 8> zeroArray;
        zeroArray.fill(0);
        _winCanvas.fill(zeroArray);

        _winPattern = -1;
        _winPile = nullptr;
        _numWinPatternsDone = 0;
    }

    virtual void OnEffect() override {
        Segment& segment = SEGMENT;
        segment.fill(_boardBackground);

        int maxShow = _stage==Stage::Deal ? (GetStateActiveMillis() / 500) : 100;
        int showX = _stage==Stage::Deal ? ((GetStateActiveMillis() % 500) / (500/8)) : 100;

        for (Pile* pile : _allPiles) {
            if (pile->_num == 0)
                continue;

            if (pile->_drawStaggered) {
                for (int i = 0; i < std::min(pile->_num, maxShow); ++i) {
                    if (i==maxShow-1 && pile->_x >= showX)
                        continue;

                    uint32_t colour = (i==pile->_num-1) ? _cardFace : _cardBack;
                    segment.setPixelColorXY(pile->_x, pile->_y + i, colour);
                }
            }
            else {
                uint32_t colour = pile->_faceUp ? _cardFace : _cardBack;
                segment.setPixelColorXY(pile->_x, pile->_y, colour);
            }
        }

        switch (_stage) {
        case Stage::Deal:
            if (maxShow >= 5)
                _stage = Stage::Play;
            break;

        case Stage::Play:
            if (Every(500))
                StartAction();

            if (Every(1000)) {
                bool win = true;
                for (Pile* pile : _sorted)
                    if (pile->_num!=pile->_max) {
                        win = false;
                        break;
                    }

                if (win)
                    _stage = Stage::Win;
            }
            break;

        case Stage::Win:
            typedef std::vector<std::pair<int8_t, int8_t>> Pattern;
            static const std::vector<Pattern> patterns = {
                {{0, 0}, {-1, 1}, {-2, 2,}, {-3, 3}, {-3, 4}, {-4, 5}, {-4, 6}, {-4, 7}, {-5, 7}, {-6, 7}, {-7, 7}},
                {{0, 0}, {0, 1}, {0, 2}, {-1, 3}, {-1, 4}, {-1, 5}, {-2, 6}, {-2, 7}, {-3, 6}, {-4, 6}, {-5, 7}, {-6, 7}, {-7, 7}},
                {{0, 0}, {-1, 1}, {-2, 2}, {-3, 3}, {-4, 4}, {-5, 5}, {-5, 6}, {-6, 7}, {-7, 6}, {-8, 6}},
                };
            
            if (_winPattern==-1) {
                _winPattern = random8(patterns.size());
                _winPatternStep = 0;
                _winPatternFlip = random8(2) ? true : false;
                _winPatternBrightness = _numWinPatternsDone % 2 ? 255: 64;
                ++_numWinPatternsDone;
                _winPile = _sorted[random8(_sorted.size())];
            }

            if (Every(100)) {
                auto stepPos = patterns[_winPattern][_winPatternStep];

                if (_winPatternFlip)
                    stepPos.first -= _winPile->_x;
                else
                    stepPos.first += _winPile->_x;

                bool onscreen = stepPos.first >= 0 && stepPos.first <= 7;
                if (onscreen) {
                    _winCanvas[stepPos.first][stepPos.second] = (uint32_t)(CRGB(_cardBack).nscale8(_winPatternBrightness));
                }

                ++_winPatternStep;
                if (_winPatternStep >= patterns[_winPattern].size() || !onscreen)
                    _winPattern = -1;
            }

            for (int x = 0; x < 8; ++x)
                for (int y = 0; y < 8; ++y) {
                    uint32_t col = _winCanvas[x][y];
                    if (col!=0) // Treat black as transparent
                        segment.setPixelColorXY(x, y, col);
                }

            if (_numWinPatternsDone >= 8 && Every(1000))
                RandomTransition();
            break;
        }
    }

private:
    struct Pile {
        int _max = 0;
        int _num = 0;
        bool _acceptMultiMove = false;
        bool _multiMove = false;
        bool _drawStaggered = false;
        bool _faceUp = false;

        int _x;
        int _y;
    };

    void StartAction() {
        enum class Action : int {
            DeckToReveal,
            RevealToBoard,
            RevealToSorted,
            BoardToBoard,
            BoardToSorted,

            Noof,
        };

        static const uint8_t probabilities[] = {5, 4, 1, 8, 6}; // Probabilities of each action type.
        static_assert((sizeof(probabilities)/sizeof(probabilities[0]))==(int)Action::Noof, "Probability table has the wrong number of items");

        static const std::array<uint8_t, (int)Action::Noof> runningProbabilities = [&](){
            std::array<uint8_t, (int)Action::Noof> totals;
            uint8_t total = 0;
            int num = 0;
            for (uint8_t prob : probabilities) {
                total += prob;
                totals[num] = total;
                ++num;
            }
            return totals;
        }();

        for (int attempt = 0; attempt < 5; ++attempt) {
            // Choose an action by weighted random
            uint8_t randProbability = random8(runningProbabilities.back());
            uint8_t randItemNum = std::upper_bound(runningProbabilities.begin(), runningProbabilities.end(), randProbability) - runningProbabilities.begin();

            Action action = (Action)randItemNum;
            switch(action) {
                case Action::DeckToReveal:
                    if (!StartActionFromPiles({&_deck}, {&_revealed}))
                        continue;
                    break;
                case Action::RevealToBoard:
                    if (!StartActionFromPiles({&_revealed}, _board))
                        continue;
                    break;
                case Action::RevealToSorted:
                    if (!StartActionFromPiles({&_revealed}, _sorted))
                        continue;
                    break;
                case Action::BoardToBoard:
                    if (!StartActionFromPiles(_board, _board))
                        continue;
                    break;
                case Action::BoardToSorted:
                    if (!StartActionFromPiles(_board, _sorted))
                        continue;
                    break;
                case Action::Noof:
                    break;
            }

            break; // StartActionFromPiles returned success - exit the loop
        }
    }

    bool StartActionFromPiles(const std::vector<Pile*>& piles, const std::vector<Pile*>& targetPiles) {
        Pile* src = nullptr;
        Pile* dest = nullptr;
        int numToMove = 0;

        int srcAttempts = 20;
        bool success = false;
        do {
            --srcAttempts;

            Pile* pile = piles[random8(piles.size())];
            if (pile->_num == 0)
                continue;
            
            int targetAttempts = 3;
            do {
                --targetAttempts;

                Pile* target = targetPiles[random8(targetPiles.size())];
                if (target==pile)
                    continue;
                if (target->_num >= target->_max && target->_max > 0)
                    continue;

                src = pile;
                dest = target;
                numToMove = 1;

                if (pile->_multiMove && target->_multiMove) {
                    if (pile->_num >= 2 && (target->_max - target->_num >= 2)) {
                        numToMove = random8(3);
                    }
                }

                success = true;
                break;
            } while (targetAttempts > 0);
        } while(!success && srcAttempts > 0);

        if (src && dest && numToMove > 0) {
            src->_num -= numToMove;
            dest->_num += numToMove;

            return true;
        }

        return false;
    }

    uint32_t _boardBackground = RGBW32(0, 64, 0, 0);
    uint32_t _cardFace = RGBW32(255, 255, 255, 0);
    uint32_t _cardBack = RGBW32(255, 0, 0, 0);

    Pile _deck;
    Pile _revealed;
    std::vector<Pile*> _sorted;
    std::vector<Pile*> _board;

    std::vector<Pile*> _allPiles;
    std::array<std::array<uint32_t, 8>, 8> _winCanvas;

    int _winPattern;
    bool _winPatternFlip;
    int _winPatternStep;
    uint8_t _winPatternBrightness;
    Pile* _winPile;
    int _numWinPatternsDone;

    enum class Stage {
        Deal,
        Play,
        Win
    };

    Stage _stage;
};