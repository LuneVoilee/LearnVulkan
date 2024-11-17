#pragma once

namespace Math
{
    template <typename T>
    T Clamp(T value , T min , T max)
    {
        return value < min ? min : ( value > max ? max : value );
    };
}
