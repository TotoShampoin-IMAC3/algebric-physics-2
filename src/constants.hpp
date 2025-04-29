#pragma once

#include "klein/point.hpp"

const float WIDTH = 1280.f;
const float HEIGHT = 960.f;

const float NEAR = 0.1f;
const float FAR = 1000.f;

const int N = 16;
const float KNOT = 1.f;
const float STIFF = 3000.f;
const float MASS = 1.f;
const float VISCOSITY = 2.5f;
const float GRAVITY = 5.0f;

const float PINCH_FORCE = 100000.0f;

const float POINT_SIZE = 0.05f;
const float LINE_SIZE = 0.025f;
const float PLANE_SIZE = 100.0f;

const float DENSITY_REPULSION = 500.f;
const float DENSITY_LOOKUP_RADIUS = 2.f;
const float DENSITY_GRID_SIZE = 1.f;

const kln::point WIND_AMP(0, 10, 10);
const kln::point WIND_FREQ(0, 5, 0.5);
