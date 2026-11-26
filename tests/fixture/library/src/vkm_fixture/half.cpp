#include <cassert>

int vkmFixtureHalf(int even) {
    assert(even % 2 == 0);
    return even / 2;
}
