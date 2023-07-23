#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
    // Two global constant variables `LIVE` and `DEAD` are assigned with integer values to represent the status of living cell and dead cell.
    constexpr const char LIVE = 1;
    constexpr const char DEAD = 0;
    // One global constant variable `BORDER_SIZE` is assigned with integer value that represent the size of grid border.
    constexpr const int BORDER_SIZE = 2;
}

#endif