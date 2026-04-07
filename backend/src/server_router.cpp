#include "../include/server_router.h"
#include <iostream>

using json = nlohmann::json;

namespace server {

    ServerRouter::ServerRouter(const std::string& staticFilesPath) : frontendPath(staticFilesPath) {
        database = db::Database::getInstance();
        setupRoutes();
    }

    void ServerRouter::setupCORS(httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

    void ServerRouter::setupRoutes() {
        svr.set_mount_point("/", frontendPath);

        // Options preflight
        svr.Options(R"(.*)", [this](const httplib::Request&, httplib::Response& res) {
            setupCORS(res);
            res.status = 200;
        });

        // Auth routes
        svr.Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) { handleLogin(req, res); });
        svr.Post("/api/register", [this](const httplib::Request& req, httplib::Response& res) { handleRegister(req, res); });
        
        // Data routes
        svr.Get(R"(/api/user/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetUserProfile(req, res); });
        svr.Get(R"(/api/dashboard/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetDashboard(req, res); });
        svr.Get(R"(/api/analytics/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleAnalytics(req, res); });
        
        // Account routing
        svr.Post("/api/account", [this](const httplib::Request& req, httplib::Response& res) { handleCreateAccount(req, res); });
        svr.Get(R"(/api/accounts/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetAccounts(req, res); });
        svr.Put("/api/account/edit", [this](const httplib::Request& req, httplib::Response& res) { handleEditAccount(req, res); });
        svr.Delete(R"(/api/account/delete/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleDeleteAccount(req, res); });

        // Transaction routing
        svr.Post("/api/transaction", [this](const httplib::Request& req, httplib::Response& res) { handleAddTransaction(req, res); });
        svr.Get(R"(/api/transactions/all/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetAllTransactions(req, res); });
        svr.Get(R"(/api/transactions/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetTransactions(req, res); });
        svr.Post("/api/transaction/edit", [this](const httplib::Request& req, httplib::Response& res) { handleEditTransaction(req, res); });
        svr.Delete(R"(/api/transaction/delete/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleDeleteTransaction(req, res); });

        // Card Routing
        svr.Post("/api/card", [this](const httplib::Request& req, httplib::Response& res) { handleCreateCard(req, res); });
        svr.Get(R"(/api/cards/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetCards(req, res); });
        
        // Budget Routing
        svr.Post("/api/budget/set", [this](const httplib::Request& req, httplib::Response& res) { handleSetBudget(req, res); });
        svr.Post("/api/budget/allocate", [this](const httplib::Request& req, httplib::Response& res) { handleAllocateBudget(req, res); });
        svr.Get(R"(/api/budget/analysis/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetBudgetAnalysis(req, res); });
        svr.Put("/api/budget/edit", [this](const httplib::Request& req, httplib::Response& res) { handleEditBudget(req, res); });
        svr.Delete(R"(/api/budget/delete/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleDeleteBudget(req, res); });

        // Alerts & News Routing
        svr.Get(R"(/api/alerts/(.*))", [this](const httplib::Request& req, httplib::Response& res) { handleGetAlerts(req, res); });
        svr.Get("/api/news", [this](const httplib::Request& req, httplib::Response& res) { handleGetNews(req, res); });
    }

    void ServerRouter::handleGetUserProfile(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        auto user = database->getUser(userId);
        if (user) {
            json j = {{"success", true}, {"user", user->toJson()}};
            res.set_content(j.dump(), "application/json");
        } else {
            res.status = 404;
        }
    }

    void ServerRouter::handleAnalytics(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        
        auto accs = database->getAccountsForUser(userId);
        
        std::map<int, double> catTotals;
        json accStats = json::array();

        for(auto& a : accs) {
            accStats.push_back({{"name", a.getAccountName()}, {"balance", a.getBalance()}, {"type", static_cast<int>(a.getType())}});
            
            auto txs = database->getTransactionsForAccount(a.getId());
            for (auto& t : txs) {
                if(t.getType() == models::TransactionType::EXPENSE) {
                    catTotals[static_cast<int>(t.getCategory())] += t.getAmount();
                }
            }
        }

        json catJson = json::object();
        for(auto& pair : catTotals) {
            catJson[std::to_string(pair.first)] = pair.second;
        }

        json resJson = {
            {"success", true},
            {"accountsData", accStats},
            {"categoryData", catJson}
        };
        res.set_content(resJson.dump(), "application/json");
    }

    void ServerRouter::handleLogin(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto reqJson = json::parse(req.body);
            std::string username = reqJson.at("username");
            std::string password = reqJson.at("password");

            auto user = database->getUserByUsername(username);
            if (user && user->verifyPassword(password)) {
                json resJson = {
                    {"success", true},
                    {"message", "Login successful"},
                    {"userId", user->getId()}
                };
                res.set_content(resJson.dump(), "application/json");
            } else {
                json err = {{"success", false}, {"error", "Invalid credentials"}};
                res.status = 401;
                res.set_content(err.dump(), "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"success":false, "error":"Invalid Request"})", "application/json");
        }
    }

    void ServerRouter::handleRegister(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto reqJson = json::parse(req.body);
            std::string username = reqJson.at("username");
            std::string password = reqJson.at("password");
            std::string email = reqJson.at("email");
            std::string name = reqJson.at("fullName");

            if (database->getUserByUsername(username)) {
                res.status = 409;
                res.set_content(R"({"success":false, "error":"Username exists"})", "application/json");
                return;
            }

            models::User newUser(username, password, email, name);
            database->addUser(newUser);
            
            // Create a default Cash account
            models::Account defaultAcc(newUser.getId(), "Main Wallet", models::AccountType::CASH, 0.0);
            database->addAccount(defaultAcc);
            
            database->persist();

            json resJson = {{"success", true}, {"userId", newUser.getId()}};
            res.set_content(resJson.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"success":false, "error":"Invalid Request"})", "application/json");
        }
    }

    void ServerRouter::handleGetDashboard(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        if (userId.empty()) return;

        auto accs = database->getAccountsForUser(userId);
        double totalBal = 0.0;
        json arr = json::array();
        for(auto& a : accs) {
            totalBal += a.getBalance();
            arr.push_back(a.toJson());
        }

        json resJson = {
            {"success", true},
            {"totalBalance", totalBal},
            {"accounts", arr}
        };
        res.set_content(resJson.dump(), "application/json");
    }

    void ServerRouter::handleCreateAccount(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto r = json::parse(req.body);
            std::string userId = r.at("userId");
            std::string name = r.at("name");
            int typeInt = r.at("type");
            double bal = r.at("balance");

            models::Account newAcc(userId, name, static_cast<models::AccountType>(typeInt), bal);
            database->addAccount(newAcc);
            database->persist();

            json j = {{"success", true}, {"account", newAcc.toJson()}};
            res.set_content(j.dump(), "application/json");
        } catch(...) {
            res.status = 400;
        }
    }

    void ServerRouter::handleGetAccounts(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        auto accs = database->getAccountsForUser(userId);
        json arr = json::array();
        for(auto& a : accs) arr.push_back(a.toJson());
        json j = {{"success", true}, {"accounts", arr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleAddTransaction(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto r = json::parse(req.body);
            std::string accId = r.at("accountId");
            double amount = r.at("amount");
            int type = r.at("type");
            int cat = r.at("category");
            std::string desc = r.at("description");

            auto account = database->getAccount(accId);
            if (!account) { res.status = 404; return; }

            if (type == 1) { // EXPENSE
                if(!account->withdraw(amount)) {
                    res.status = 400;
                    res.set_content(R"({"success":false, "error":"Insufficient funds"})", "application/json");
                    return;
                }
            } else if (type == 0) { // INCOME
                account->deposit(amount);
            }

            models::Transaction t(accId, amount, static_cast<models::TransactionType>(type), static_cast<models::Category>(cat), desc);
            database->addTransaction(t);
            database->updateAccount(*account);
            database->persist();

            json j = {{"success", true}, {"transaction", t.toJson()}};
            res.set_content(j.dump(), "application/json");
        } catch(std::exception& e) {
            std::cout << e.what() << "\n";
            res.status = 400;
        }
    }

    void ServerRouter::handleGetTransactions(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string accountId = req.matches[1];
        auto txs = database->getTransactionsForAccount(accountId);
        json arr = json::array();
        for(auto& t : txs) arr.push_back(t.toJson());
        
        json j = {{"success", true}, {"transactions", arr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleCreateCard(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto r = json::parse(req.body);
            std::string accId = r.at("accountId");
            std::string holder = r.at("holderName");
            double limit = r.at("limit");

            models::VirtualCard newCard(accId, holder, limit);
            database->addVirtualCard(newCard);
            database->persist();

            json j = {{"success", true}, {"card", newCard.toJson()}};
            res.set_content(j.dump(), "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleGetCards(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string accountId = req.matches[1];
        auto cards = database->getCardsForAccount(accountId);
        json arr = json::array();
        for(auto& c : cards) arr.push_back(c.toJson());
        
        json j = {{"success", true}, {"cards", arr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleSetBudget(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto r = json::parse(req.body);
            std::string userId = r.at("userId");
            auto items = r.at("items");

            for(auto& item : items) {
                int catInt = item.at("category");
                double limit = item.at("limit");
                std::string catName = "Budget"; // fallback
                if(catInt == 10) catName = "Investment";
                else if(catInt == 5) catName = "Savings";
                else if(catInt == 4) catName = "EMI";
                else if(catInt == 2) catName = "Essentials";
                else if(catInt == 11) catName = "Insurance";
                else if(catInt == 6) catName = "Other";
                
                std::string bName = item.value("name", catName);
                
                models::Budget b(userId, bName, static_cast<models::Category>(catInt), limit, 0, "Monthly");
                database->addBudget(b);
            }
            database->persist();
            res.set_content(R"({"success":true})", "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleAllocateBudget(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto r = json::parse(req.body);
            std::string userId = r.at("userId");
            double salary = r.at("monthlySalary");

            models::Budget bEMI(userId, "EMI Allocation", models::Category::EMI, salary * 0.20, 0, "Monthly");
            models::Budget bSave(userId, "Savings Goal", models::Category::SAVINGS, salary * 0.20, 0, "Monthly");
            models::Budget bInvest(userId, "Primary Investments", models::Category::INVESTMENT, salary * 0.15, 0, "Monthly");
            models::Budget bInsure(userId, "Insurance Policy", models::Category::INSURANCE, salary * 0.05, 0, "Monthly");
            models::Budget bExp(userId, "Monthly Essentials", models::Category::ESSENTIALS, salary * 0.30, 0, "Monthly");
            models::Budget bOther(userId, "Miscellaneous", models::Category::OTHER, salary * 0.10, 0, "Monthly");

            database->addBudget(bEMI);
            database->addBudget(bSave);
            database->addBudget(bInvest);
            database->addBudget(bInsure);
            database->addBudget(bExp);
            database->addBudget(bOther);
            
            database->persist();

            json j = {{"success", true}, {"message", "Allocated successfully"}};
            res.set_content(j.dump(), "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleGetBudgetAnalysis(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        
        auto budgets = database->getBudgetsForUser(userId);
        
        json arr = json::array();
        for(auto& b : budgets) {
            if(b.getUserId() == userId) {
                arr.push_back(b.toJson());
            }
        }
        
        json j = {{"success", true}, {"budgets", arr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleGetAlerts(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        auto alerts = database->getAlertsForUser(userId);
        
        if (alerts.empty()) {
            models::Alert a1(userId, "Credit Card Bill due in 3 days ($45.00)", models::AlertType::DUE);
            models::Alert a2(userId, "You've exceeded 80% of your Essentials budget.", models::AlertType::WARNING);
            models::Alert a3(userId, "New Govt Scheme 'Green Bonds' offers 8.5% p.a.", models::AlertType::NEWS);
            database->addAlert(a1);
            database->addAlert(a2);
            database->addAlert(a3);
            database->persist();
            alerts.push_back(a1);
            alerts.push_back(a2);
            alerts.push_back(a3);
        }

        json arr = json::array();
        for(auto& a : alerts) arr.push_back(a.toJson());
        
        json j = {{"success", true}, {"alerts", arr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleGetNews(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        json newsArr = json::array({
            {{"title", "TechStock surged 5%"}, {"description", "Significant trading volume observed. Expected ROI: +12% this quarter."}, {"type", "SHARE"}},
            {{"title", "National Solar Bonds"}, {"description", "Government-backed bonds yielding 7.5% annually. Low risk."}, {"type", "SCHEME"}},
            {{"title", "Corporate Alpha Debentures"}, {"description", "Private sector bonds released recently. Expected yield: 9.2%."}, {"type", "BOND"}},
            {{"title", "Saving Tip: 50/30/20 Rule"}, {"description", "Allocate 50% to needs, 30% to wants, 20% to savings for a balanced financial life."}, {"type", "TIP"}}
        });

        json j = {{"success", true}, {"news", newsArr}};
        res.set_content(j.dump(), "application/json");
    }

    void ServerRouter::handleGetAllTransactions(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string userId = req.matches[1];
        auto accounts = database->getAccountsForUser(userId);
        
        json response;
        response["success"] = true;
        response["transactions"] = json::array();
        
        for (const auto& acc : accounts) {
            auto txs = database->getTransactionsForAccount(acc.getId());
            for (const auto& tx : txs) {
                json txJson = tx.toJson();
                txJson["accountName"] = acc.getAccountName(); // contextual view
                response["transactions"].push_back(txJson);
            }
        }
        res.set_content(response.dump(), "application/json");
    }

    void ServerRouter::handleEditTransaction(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto j = json::parse(req.body);
            std::string txId = j.at("id");
            double newAmount = j.at("amount");
            int newCategory = j.at("category");
            std::string newDesc = j.at("description");

            models::Transaction* tx = database->getTransaction(txId);
            if (!tx) { res.status = 404; return; }
            
            models::Account* acc = database->getAccount(tx->getAccountId());
            if (!acc) { res.status = 404; return; }

            // Reverse old effect
            if (tx->getType() == models::TransactionType::INCOME) {
                acc->withdraw(tx->getAmount());
            } else if (tx->getType() == models::TransactionType::EXPENSE) {
                acc->deposit(tx->getAmount());
            }

            // Apply new effect
            if (tx->getType() == models::TransactionType::INCOME) {
                acc->deposit(newAmount);
            } else if (tx->getType() == models::TransactionType::EXPENSE) {
                if(!acc->withdraw(newAmount)) {
                    acc->withdraw(tx->getAmount()); // Restitution
                    res.set_content(R"({"success":false, "error":"Insufficient funds"})", "application/json");
                    return;
                }
            }

            tx->setAmount(newAmount);
            tx->setCategory(static_cast<models::Category>(newCategory));
            tx->setDescription(newDesc);

            database->updateTransaction(*tx);
            database->updateAccount(*acc);
            database->persist();

            json resJ = {{"success", true}};
            res.set_content(resJ.dump(), "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleEditAccount(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto j = json::parse(req.body);
            std::string accId = j.at("id");
            std::string newName = j.at("name");

            models::Account* acc = database->getAccount(accId);
            if (!acc) { res.status = 404; return; }

            // Account constructor is used for creation, but we don't have setName in Account
            // Let's modify the old JSON and construct a new one, or we need setName in Account
            // Wait, models::Account doesn't have setName.
            // I'll update it through JSON hack or just return successfully since it's a structural limitation of models.h right now. Let's assume we can fetch its JSON, update name, and fromJson.
            json accJ = acc->toJson();
            accJ["accountName"] = newName;
            acc->fromJson(accJ);

            database->updateAccount(*acc);
            database->persist();

            json resJ = {{"success", true}};
            res.set_content(resJ.dump(), "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleDeleteAccount(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string accId = req.matches[1];
        if (database->deleteAccount(accId)) {
            database->persist();
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"success":false})", "application/json");
        }
    }

    void ServerRouter::handleDeleteTransaction(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string txId = req.matches[1];
        
        models::Transaction* tx = database->getTransaction(txId);
        if(tx) {
            models::Account* acc = database->getAccount(tx->getAccountId());
            if (acc) {
                // Reverse transaction effect
                if (tx->getType() == models::TransactionType::INCOME) {
                    acc->withdraw(tx->getAmount());
                } else if (tx->getType() == models::TransactionType::EXPENSE) {
                    acc->deposit(tx->getAmount());
                }
                database->updateAccount(*acc);
            }
            if (database->deleteTransaction(txId)) {
                database->persist();
                res.set_content(R"({"success":true})", "application/json");
                return;
            }
        }
        res.status = 404;
        res.set_content(R"({"success":false})", "application/json");
    }

    void ServerRouter::handleEditBudget(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        try {
            auto j = json::parse(req.body);
            std::string bId = j.at("id");
            std::string newName = j.value("name", "");
            double newLimit = j.at("limitAmount");

            models::Budget* b = database->getBudget(bId);
            if (!b) { res.status = 404; return; }

            if (!newName.empty()) b->setName(newName);
            b->setLimitAmount(newLimit);

            database->updateBudget(*b);
            database->persist();

            json resJ = {{"success", true}};
            res.set_content(resJ.dump(), "application/json");
        } catch(...) { res.status = 400; }
    }

    void ServerRouter::handleDeleteBudget(const httplib::Request& req, httplib::Response& res) {
        setupCORS(res);
        std::string bId = req.matches[1];
        if (database->deleteBudget(bId)) {
            database->persist();
            res.set_content(R"({"success":true})", "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"success":false})", "application/json");
        }
    }

    void ServerRouter::start(int port) {
        std::cout << "Starting REST API server on port " << port << "...\n";
        svr.listen("0.0.0.0", port);
    }

} // namespace server
