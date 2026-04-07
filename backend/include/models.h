#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <memory>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

namespace models {

    // Forward declarations
    class Account;
    class VirtualCard;
    class Transaction;
    class Budget;
    class Goal;
    class Subscription;
    class Loan;
    class Alert;

    // --- ENUMS ---
    enum class AccountType { BANK, CASH, CREDIT, DEBIT };
    enum class TransactionType { INCOME, EXPENSE, TRANSFER };
    enum class Category { FOOD, TRANSPORT, ESSENTIALS, HEALTH, EMI, SAVINGS, OTHER, RENT, ENTERTAINMENT, UTILITIES, INVESTMENT, INSURANCE, SALARY, BONUS };
    enum class SubscriptionStatus { ACTIVE, PAUSED, CANCELLED };
    enum class AlertType { DUE, REMINDER, NEWS, WARNING };

    // --- BASE ENTITY ---
    class Entity {
    protected:
        std::string id;
        std::time_t createdAt;
        std::time_t updatedAt;
    public:
        Entity();
        virtual ~Entity() = default;
        
        std::string getId() const;
        void setId(const std::string& newId);
        
        std::time_t getCreatedAt() const;
        std::time_t getUpdatedAt() const;
        void updateTimestamp();

        virtual json toJson() const = 0;
        virtual void fromJson(const json& j) = 0;
    };

    // --- USER ---
    class User : public Entity {
    private:
        std::string username;
        std::string passwordHash;
        std::string email;
        std::string fullName;
        bool isPremium;
    public:
        User();
        User(std::string uname, std::string pwdHash, std::string mail, std::string name);
        
        std::string getUsername() const;
        void setUsername(const std::string& uname);
        
        std::string getPasswordHash() const;
        void setPasswordHash(const std::string& hash);
        
        std::string getEmail() const;
        bool verifyPassword(const std::string& pwd) const;
        void upgradeToPremium();
        bool getIsPremium() const;

        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- ACCOUNT ---
    class Account : public Entity {
    private:
        std::string userId;
        std::string accountName;
        AccountType type;
        double balance;
        std::string currency;
    public:
        Account();
        Account(std::string uId, std::string name, AccountType t, double bal, std::string curr = "USD");
        
        std::string getUserId() const;
        std::string getAccountName() const;
        AccountType getType() const;
        double getBalance() const;
        std::string getCurrency() const;

        void deposit(double amount);
        bool withdraw(double amount);
        
        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- VIRTUAL CARD ---
    class VirtualCard : public Entity {
    private:
        std::string accountId;
        std::string cardNumber;
        std::string cardHolderName;
        std::string expiryDate;
        std::string cvv;
        double dailyLimit;
        double currentDailySpend;
        bool isFrozen;
    public:
        VirtualCard();
        VirtualCard(std::string accId, std::string holder, double limit);
        
        std::string getAccountId() const;
        std::string getCardNumber() const;
        double getDailyLimit() const;
        bool getIsFrozen() const;
        
        void toggleFreeze();
        bool processPayment(double amount);
        void resetDailySpend();

        json toJson() const override;
        void fromJson(const json& j) override;
    private:
        std::string generateCardNumber();
        std::string generateCVV();
        std::string generateExpiryDate();
    };

    // --- TRANSACTION ---
    class Transaction : public Entity {
    private:
        std::string accountId;
        std::string linkedCardId; // optional
        double amount;
        TransactionType type;
        Category category;
        std::string description;
        std::string targetAccountId; // For transfers
    public:
        Transaction();
        Transaction(std::string accId, double amt, TransactionType t, Category cat, std::string desc);
        
        std::string getAccountId() const;
        double getAmount() const;
        TransactionType getType() const;
        Category getCategory() const;
        std::string getDescription() const;

        void setAmount(double amt);
        void setCategory(Category cat);
        void setDescription(const std::string& desc);

        void setLinkedCardId(const std::string& cardId);
        void setTargetAccountId(const std::string& tAccId);

        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- BUDGET ---
    class Budget : public Entity {
    private:
        std::string userId;
        std::string name;
        Category category;
        double limitAmount;
        double currentSpent;
        int targetValue; // stores month, week id, or year
        std::string periodType; // "Weekly", "Monthly", "Yearly"
    public:
        Budget();
        Budget(std::string uId, std::string n, Category cat, double limit, int targetVal, std::string period);
        
        std::string getUserId() const;
        std::string getName() const;
        Category getCategory() const;
        double getLimitAmount() const;
        double getCurrentSpent() const;
        
        void setName(const std::string& newName);
        void setLimitAmount(double newLimit);
        void addExpense(double amount);
        bool isExceeded() const;
        double getRemaining() const;

        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- GOAL ---
    class Goal : public Entity {
    private:
        std::string userId;
        std::string goalName;
        double targetAmount;
        double currentSaved;
        std::time_t deadline;
    public:
        Goal();
        Goal(std::string uId, std::string name, double target, std::time_t dead);
        
        void addFunds(double amount);
        double getProgressPercentage() const;
        bool isAchieved() const;

        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- SUBSCRIPTION ---
    class Subscription : public Entity {
    private:
        std::string accountId;
        std::string serviceName;
        double costPerBillingCycle;
        std::time_t nextBillingDate;
        SubscriptionStatus status;
    public:
        Subscription();
        Subscription(std::string accId, std::string service, double cost, std::time_t nextBill);
        
        bool processBilling(double& outAmount);
        void cancel();
        void pause();
        void resume();

        json toJson() const override;
        void fromJson(const json& j) override;
    };

    // --- ALERT ---
    class Alert : public Entity {
    private:
        std::string userId;
        std::string message;
        AlertType type;
        bool isRead;
    public:
        Alert();
        Alert(std::string uId, std::string msg, AlertType t);
        
        std::string getUserId() const;
        std::string getMessage() const;
        AlertType getType() const;
        bool getIsRead() const;
        
        void markAsRead();

        json toJson() const override;
        void fromJson(const json& j) override;
    };

} // namespace models
