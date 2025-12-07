#pragma once
#include <ctime>
#include <glm/detail/setup.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

template <class T>
bool valid_float(const T& x) {
    return !glm::isnan(x) && !glm::isinf(x);
}
bool valid_vec2(const glm::vec2& x) { return valid_float(x[0]) && valid_float(x[1]); }

double oriented_angle_between(const glm::vec2& a, const glm::vec2& b) {
    const glm::vec2&& _a = (a == glm::vec2(0.0f, 0.0f) || glm::isnan(a.x) || glm::isnan(a.y) || glm::isinf(a.x) || glm::isinf(a.y)) ? glm::vec2(0.0f, 1.0f) : a;
    const glm::vec2&& _b = (b == glm::vec2(0.0f, 0.0f) || glm::isnan(b.x) || glm::isnan(b.y) || glm::isinf(b.x) || glm::isinf(b.y)) ? glm::vec2(0.0f, 1.0f) : b;
    return glm::orientedAngle(glm::normalize(_a), glm::normalize(_b));
}
double angle_between(const glm::vec2& a, const glm::vec2& b) {
    const glm::vec2&& _a = (a == glm::vec2(0.0f, 0.0f) || glm::isnan(a.x) || glm::isnan(a.y) || glm::isinf(a.x) || glm::isinf(a.y)) ? glm::vec2(0.0f, 1.0f) : a;
    const glm::vec2&& _b = (b == glm::vec2(0.0f, 0.0f) || glm::isnan(b.x) || glm::isnan(b.y) || glm::isinf(b.x) || glm::isinf(b.y)) ? glm::vec2(0.0f, 1.0f) : b;
    return glm::angle(glm::normalize(_a), glm::normalize(_b));
}

template <glm::length_t L, typename T, glm::qualifier Q>
glm::vec<L, T, Q> limit_length(const glm::vec<L, T, Q>& v, T max_len) {
    T len = glm::length(v);
    if (len > max_len) return v * (max_len / len);
    return v;
}

struct PID {
private:
    double _prev_error = 0.0f;
    double _I = 0.0f;

public:
    double kP;
    double kI;
    double kD;
    double step(const double& error, const double& dt) {
        _I = _I + error * dt;
        const double prev_error = _prev_error;
        _prev_error = error;
        return kP * error + kI * _I + kD * (error - prev_error) / dt;
    }
    PID(const double& kP, const double& kI, const double& kD) : kP(kP), kI(kI), kD(kD) {}
};
