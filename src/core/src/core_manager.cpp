#include <maat_core/core_manager.h>
#include <iostream>

namespace maat {
namespace core {

CoreManager::CoreManager() {
    std::cout << "[CoreManager] Constructed" << std::endl;
}

CoreManager::~CoreManager() {
    std::cout << "[CoreManager] Destructed" << std::endl;
}

} // namespace core
} // namespace maat
