//
// Created by Danh Thien Luu on 20.11.23.
//

#ifndef RAYTRACER_HITTABLE_H
#define RAYTRACER_HITTABLE_H

#include "geometry/geometry.h"


class Hittable {
public:
    virtual ~Hittable() = default;

    virtual bool hit(const Ray3df &r, float ray_tmin, float ray_tmax, HitRecord &h) = 0;

};

#endif //RAYTRACER_HITTABLE_H
