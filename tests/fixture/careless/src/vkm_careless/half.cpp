#include <cassert>

/// Never defined: a call the archive leaves for the profiler to answer.
void TracyFixtureMark();

int vkmCarelessHalf(int even) {
    assert(even % 2 == 0);
    TracyFixtureMark();
    return even / 2;
}
