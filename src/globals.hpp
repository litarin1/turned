#pragma once

// made for Box2D: they say its works better when objects are near 1.0 unit size.
// the more ZOOM_FACTOR is, the less all physics size is
constexpr double ZOOM_FACTOR = 64.0;
// TODO: 60Hz timer for physics
constexpr double PHYSICS_RATE = 60.0;
// Box2D
constexpr int PHYSICS_SUBSTEPS_COUNT = 4;
