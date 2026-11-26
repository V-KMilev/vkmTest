// Tests whose files, as #line names them, sit in folders under tests/: each is listed in the area of the folder under the last tests/ in its path.

#include "vkm_test/check.h"

#line 100 "/game/tests/render/opengl/gpu.cpp"
TEST("a test in a folder below its area") {}

#line 200 "C:\\game\\tests\\physics\\contacts.cpp"
TEST("a test whose path takes backslashes") {}

#line 300 "/game/tests/fixture/tests/net/lockstep.cpp"
TEST("a test under two tests folders, in the last one's area") {}
