#pragma once
namespace ArgusMaze
{
    enum class State
    {
        FollowLine,
        AdvanceToCenter,
        Brake,
        TurnRight,
        Stopped
    };
    void begin();
    void update();
}