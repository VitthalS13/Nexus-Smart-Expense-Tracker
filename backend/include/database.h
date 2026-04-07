#pragma once

#include "models.h"
#include <string>
#include <vector>
#include <mutex>
#include <map>

namespace db {

    class Database {
    private:
        std::string basePath;
        std::mutex dbMutex;

        // In-memory cache
        std::map<std::string, models::User> users;
        std::map<std::string, models::Account> accounts;
        std::map<std::string, models::VirtualCard> virtualCards;
        std::map<std::string, models::Transaction> transactions;
        std::map<std::string, models::Budget> budgets;
        std::map<std::string, models::Alert> alerts;

        // Singleton instance
        static Database* instance;

        Database(const std::string& path);
        void loadAll();
        void saveAll();

        template <typename T>
        void saveMapToFile(const std::map<std::string, T>& dataMap, const std::string& filename);

        template <typename T>
        void loadMapFromFile(std::map<std::string, T>& dataMap, const std::string& filename);

    public:
        // Singleton Accessor
        static Database* getInstance();
        static void init(const std::string& path);

        // --- Users ---
        void addUser(const models::User& user);
        models::User* getUser(const std::string& id);
        models::User* getUserByUsername(const std::string& username);
        std::vector<models::User> getAllUsers();

        // --- Accounts ---
        void addAccount(const models::Account& account);
        models::Account* getAccount(const std::string& id);
        std::vector<models::Account> getAccountsForUser(const std::string& userId);
        void updateAccount(const models::Account& account);
        bool deleteAccount(const std::string& id);

        // --- Virtual Cards ---
        void addVirtualCard(const models::VirtualCard& card);
        models::VirtualCard* getVirtualCard(const std::string& id);
        std::vector<models::VirtualCard> getCardsForAccount(const std::string& accountId);
        void updateVirtualCard(const models::VirtualCard& card);
        bool deleteVirtualCard(const std::string& id);

        // --- Transactions ---
        void addTransaction(const models::Transaction& transaction);
        models::Transaction* getTransaction(const std::string& id);
        std::vector<models::Transaction> getTransactionsForAccount(const std::string& accountId);
        std::vector<models::Transaction> getAllTransactions();
        void updateTransaction(const models::Transaction& transaction);
        bool deleteTransaction(const std::string& id);

        // --- Budgets ---
        void addBudget(const models::Budget& budget);
        models::Budget* getBudget(const std::string& id);
        std::vector<models::Budget> getBudgetsForUser(const std::string& userId);
        void updateBudget(const models::Budget& budget);
        bool deleteBudget(const std::string& id);

        // --- Alerts ---
        void addAlert(const models::Alert& alert);
        std::vector<models::Alert> getAlertsForUser(const std::string& userId);
        models::Alert* getAlert(const std::string& id);

        // Persistence trigger
        void persist();
    };

} // namespace db
