#include <iostream>
#include <memory>
#include <csignal>
#include <exception>

#include "maat_core/core_manager.h"
#include "maat_core/maat_mediator.h"
#include "maat_platform_windows/windows_platform_manager.h"

static maat::core::MaatMediator* g_mediator = nullptr;

// Signal handler to trigger shutdown via mediator
void signalHandler(int signum) {
    std::cout << "\nSignal (" << signum << ") received. Shutting down...\n";
    if (g_mediator) {
        g_mediator->shutdown();
    }
}

int main() {
    // Register SIGINT handler (Ctrl+C)
    std::signal(SIGINT, signalHandler);

    std::cout << "Creating components..." << std::endl;
    auto mediator = std::make_unique<maat::core::MaatMediator>();
    g_mediator = mediator.get();

    auto platformManager = std::make_unique<maat::platform::WindowsPlatformManager>(*mediator);
    auto coreManager     = std::make_unique<maat::core::CoreManager>();

    std::cout << "Registering components with mediator..." << std::endl;
    mediator->registerPlatformManager(*platformManager);
    mediator->registerCoreManager(*coreManager);

    std::cout << "Initializing Maat via Mediator..." << std::endl;
    try {
        mediator->initialize();
    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Running Maat via Mediator... Press Ctrl+C to exit." << std::endl;
    try {
        mediator->run();
    } catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Maat finished." << std::endl;
    return 0;
}
