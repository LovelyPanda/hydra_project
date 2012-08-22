#include "math/AABB.hpp"
#include <iostream>

using namespace hydra::math;

int main(){

    AABB aabb(Vector3D(0.0, 0.0, 0.0), Vector3D(1.0, 1.0, 1.0));

#define TEST_AABB(x, y, z) std::cout << "distance to " << x << ' ' << y << ' ' << z <<  \
    " is " << aabb.getShortestDistance(Vector3D(x, y, z)) << std::endl;

    TEST_AABB(1.0, 1.0, 1.0);
    TEST_AABB(2.0, 0.0, 0.0);
    TEST_AABB(0.0, 2.0, 0.0);
    TEST_AABB(2.0, 2.0, 2.0);
    TEST_AABB(-1.0, 0.0, 0.0);
    TEST_AABB(1.5, 0.5, 0.5);
    TEST_AABB(0.5, 1.5, 0.5);
    TEST_AABB(0.5, 0.0, 0.0);

    return 0;
}
