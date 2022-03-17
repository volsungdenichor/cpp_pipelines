#pragma once

#define DEFINE_LAMBDA(arg, ...) [&](auto&& arg) -> decltype((__VA_ARGS__)) { return (__VA_ARGS__); }
#define L(...)                  DEFINE_LAMBDA(_, __VA_ARGS__)