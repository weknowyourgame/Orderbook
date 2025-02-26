#pragma once
#include <string>

namespace TerminalColors {
    const std::string RED = "\033[1;31m";
    const std::string GREEN = "\033[1;32m";
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string CLEAR_SCREEN = "\033[2J\033[H";
} 