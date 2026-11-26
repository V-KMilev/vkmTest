#include "vkm_fixture/half.h"

#include <cassert>

#include "vkm_fixture/even.h"

int vkmFixtureHalf(int even) {
    assert(vkmFixtureEven(even));
    return even / 2;
}
