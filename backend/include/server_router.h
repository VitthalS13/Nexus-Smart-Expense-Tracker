#pragma once

#include "httplib.h"
#include "database.h"
#include "json.hpp"
#include <string>

namespace server {

    class ServerRouter {
    private:
        httplib::Server svr;
        db::Database* database;
        std::string frontendPath;

        void setupRoutes();
        void setupCORS(httplib::Response& res);

        // Auth & Profile routes
        void handleLogin(const httplib::Request& req, httplib::Response& res);
        void handleRegister(const httplib::Request& req, httplib::Response& res);
        void handleGetUserProfile(const httplib::Request& req, httplib::Response& res);
        void handleGetDashboard(const httplib::Request& req, httplib::Response& res);
        void handleAnalytics(const httplib::Request& req, httplib::Response& res);
        
        // Account Handlers
        void handleCreateAccount(const httplib::Request& req, httplib::Response& res);
        void handleGetAccounts(const httplib::Request& req, httplib::Response& res);
        void handleEditAccount(const httplib::Request& req, httplib::Response& res);
        void handleDeleteAccount(const httplib::Request& req, httplib::Response& res);
        
        // Transaction Handlers
        void handleAddTransaction(const httplib::Request& req, httplib::Response& res);
        void handleGetTransactions(const httplib::Request& req, httplib::Response& res);
        void handleGetAllTransactions(const httplib::Request& req, httplib::Response& res);
        void handleEditTransaction(const httplib::Request& req, httplib::Response& res);
        void handleDeleteTransaction(const httplib::Request& req, httplib::Response& res);

        // Card Handlers
        void handleCreateCard(const httplib::Request& req, httplib::Response& res);
        void handleGetCards(const httplib::Request& req, httplib::Response& res);
        
        // Budget Handlers
        void handleSetBudget(const httplib::Request& req, httplib::Response& res);
        void handleAllocateBudget(const httplib::Request& req, httplib::Response& res);
        void handleGetBudgetAnalysis(const httplib::Request& req, httplib::Response& res);
        void handleEditBudget(const httplib::Request& req, httplib::Response& res);
        void handleDeleteBudget(const httplib::Request& req, httplib::Response& res);

        // Alerts & News Handlers
        void handleGetAlerts(const httplib::Request& req, httplib::Response& res);
        void handleGetNews(const httplib::Request& req, httplib::Response& res);

    public:
        ServerRouter(const std::string& staticFilesPath);
        void start(int port);
    };

} // namespace server
