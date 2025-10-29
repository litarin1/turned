#pragma once
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

template <typename... T>
inline void LTRACE(const T &...args) {
    spdlog::trace(args...);
}
template <typename... T>
inline void LDEBUG(const T &...args) {
    spdlog::debug(args...);
}
template <typename... T>
inline void LINFO(const T &...args) {
    spdlog::info(args...);
}
template <typename... T>
inline void LWARN(const T &...args) {
    spdlog::warn(args...);
}
template <typename... T>
inline void LERR(const T &...args) {
    spdlog::error(args...);
}
template <typename... T>
inline void LCRIT(const T &...args) {
    spdlog::critical(args...);
}
#define LCRITRET(ret, ...)  \
    {                       \
        LCRIT(__VA_ARGS__); \
        return ret;         \
    }

inline void _init_log() {
    spdlog::set_default_logger(spdlog::stdout_color_mt("console"));
    spdlog::set_pattern("%^(%L%L)%$ %v");  // (DD) Debug message
}
