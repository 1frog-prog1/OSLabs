#pragma once

#include <string>

namespace Subprocess {

int subprocess(std::string command);

std::pair<int, int> waitProcess(int pid);

}
