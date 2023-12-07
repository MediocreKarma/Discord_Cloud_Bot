#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include "poll.h"
#include <array>
#include <atomic>
#include <thread>

std::atomic_flag func() {
    std::atomic_flag f = ATOMIC_FLAG_INIT;

    return std::move(f);
}

int main() {
    auto flag = func();
}