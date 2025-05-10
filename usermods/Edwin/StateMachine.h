#pragma once

#include <vector>

class BaseState {
public:
    friend StateMachine;

    virtual ~BaseState() {}

    virtual void Reset() {}
    virtual void OnEnter() {
        _enterTime = millis();
    }
    virtual void OnExit() {}
    virtual void OnEffect() {}

    void AddTransition(BaseState& transition) { _transitions.push_back(&transition); }
    template<typename... TStates>
    void AddTransitions(TStates &&... transitions) {
        for (BaseState* transition : {(BaseState*)&transitions...})
            AddTransition(*transition);
    }

protected:
    unsigned long GetStateActiveMillis() const { return millis() - _enterTime; }
    bool Every(int ms) const { return (GetStateActiveMillis() % ms) < FRAMETIME; }
    
    bool RandomTransitionIfMillisElapsed(int ms);
    bool RandomTransition();
    StateMachine* _sm;

private:
    unsigned long _enterTime;
    std::vector<BaseState*> _transitions;
};

class StateMachine {
public:
    template<typename T, typename... TConstructArgs>
    T& AddState(TConstructArgs &&... args) {
        std::unique_ptr<T> state = std::unique_ptr<T>(new T(args...));
        state->_sm = this;

        state->Reset();

        T* rawState = state.get();
        _states.push_back(std::move(state));

        return *rawState;
    }
    void SetState(BaseState& state) {
        if (&state==_state)
            return;
        _state = &state;
        _state->OnEnter();
    }
    void SetNextState(BaseState& state) {
        if (_nextState || &state==_state)
            return;
        
        _nextState = &state;
    }

    BaseState& GetState() const { return *_state; }

    void ResetAllStates() {
        for (auto& state : _states)
            state->Reset();
    }

    void OnEffect() {
        if (_nextState) {
            if (_state)
                _state->OnExit();
            _state = _nextState;
            _nextState = nullptr;
            if (_state)
                _state->OnEnter();
        }

        if (_state)
            _state->OnEffect();
    }

private:
    BaseState* _state = nullptr;
    BaseState* _nextState = nullptr;
    std::vector<std::unique_ptr<BaseState>> _states;
};

inline bool BaseState::RandomTransitionIfMillisElapsed(int ms) {
    if (GetStateActiveMillis() < ms)
        return false;

    return RandomTransition();    
}

inline bool BaseState::RandomTransition() {
    if (_transitions.size() == 0)
        return false;

    BaseState* transition = _transitions[random8(_transitions.size())];
    _sm->SetNextState(*transition);
    return true;
}