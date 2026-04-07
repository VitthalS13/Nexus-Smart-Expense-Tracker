#include "include/server_router.h"
#include "include/database.h"
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "========================================\n";
    std::cout << "  Smart Expense Tracker & Budget Manager \n";
    std::cout << "========================================\n";

    // Ensure data directory exists
    std::filesystem::create_directories("data");
    
    // Initialize Database singleton
    db::Database::init("data");
    std::cout << "Database initialized successfully.\n";

    // Start Server
    server::ServerRouter router("../frontend");
    router.start(8080);

    return 0;
}
