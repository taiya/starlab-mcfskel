#pragma once
#include "global.h"

class SkeletonEdge {
public:
    SkeletonNode * n1;
    SkeletonNode * n2;

    int index;

    float length;

    SkeletonEdge(SkeletonNode * N1, SkeletonNode * N2, int newIndex) {
        this->n1 = N1;
        this->n2 = N2;

        this->index = newIndex;
    }

    inline float calculateLength() {
        length = n1->distanceTo(n2);
        return length;
    }
#if 0
    inline Vec direction() {
        return this->n2->v() - this->n1->v();
    }
#endif
};
