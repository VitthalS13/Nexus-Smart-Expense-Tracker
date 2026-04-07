#include "../include/models.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace models {

    // --- Helper function to generate UUID-like strings ---
    static std::string generateId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++) ss << dis(gen);
        return ss.str();
    }

    // --- BASE ENTITY ---
    Entity::Entity() {
        this->id = generateId();
        this->createdAt = std::time(nullptr);
        this->updatedAt = this->createdAt;
    }

    std::string Entity::getId() const { return id; }
    void Entity::setId(const std::string& newId) { id = newId; }
    
    std::time_t Entity::getCreatedAt() const { return createdAt; }
    std::time_t Entity::getUpdatedAt() const { return updatedAt; }
    void Entity::updateTimestamp() { updatedAt = std::time(nullptr); }

    // --- USER ---
    User::User() : Entity(), isPremium(false) {}
    User::User(std::string uname, std::string pwdHash, std::string mail, std::string name) 
        : Entity(), username(uname), passwordHash(pwdHash), email(mail), fullName(name), isPremium(false) {}

    std::string User::getUsername() const { return username; }
    void User::setUsername(const std::string& uname) { username = uname; updateTimestamp(); }
    
    std::string User::getPasswordHash() const { return passwordHash; }
    void User::setPasswordHash(const std::string& hash) { passwordHash = hash; updateTimestamp(); }
    
    std::string User::getEmail() const { return email; }
    
    bool User::verifyPassword(const std::string& pwd) const {
        return passwordHash == pwd; // Simulation: In reality, use bcrypt
    }
    
    void User::upgradeToPremium() {
        isPremium = true;
        updateTimestamp();
    }
    
    bool User::getIsPremium() const { return isPremium; }

    json User::toJson() const {
        return {
            {"id", id},
            {"username", username},
            {"passwordHash", passwordHash},
            {"email", email},
            {"fullName", fullName},
            {"isPremium", isPremium},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void User::fromJson(const json& j) {
        if(j.contains("id")) id = j.at("id");
        if(j.contains("username")) username = j.at("username");
        if(j.contains("passwordHash")) passwordHash = j.at("passwordHash");
        if(j.contains("email")) email = j.at("email");
        if(j.contains("fullName")) fullName = j.at("fullName");
        if(j.contains("isPremium")) isPremium = j.at("isPremium");
        if(j.contains("createdAt")) createdAt = j.at("createdAt");
        if(j.contains("updatedAt")) updatedAt = j.at("updatedAt");
    }

    // --- ACCOUNT ---
    Account::Account() : Entity(), balance(0.0) {}
    Account::Account(std::string uId, std::string name, AccountType t, double bal, std::string curr)
        : Entity(), userId(uId), accountName(name), type(t), balance(bal), currency(curr) {}

    std::string Account::getUserId() const { return userId; }
    std::string Account::getAccountName() const { return accountName; }
    AccountType Account::getType() const { return type; }
    double Account::getBalance() const { return balance; }
    std::string Account::getCurrency() const { return currency; }

    void Account::deposit(double amount) {
        if(amount > 0) {
            balance += amount;
            updateTimestamp();
        }
    }

    bool Account::withdraw(double amount) {
        if(amount > 0 && balance >= amount) {
            balance -= amount;
            updateTimestamp();
            return true;
        }
        return false;
    }

    json Account::toJson() const {
        return {
            {"id", id},
            {"userId", userId},
            {"accountName", accountName},
            {"type", static_cast<int>(type)},
            {"balance", balance},
            {"currency", currency},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Account::fromJson(const json& j) {
        id = j.value("id", id);
        userId = j.value("userId", "");
        accountName = j.value("accountName", "");
        type = static_cast<AccountType>(j.value("type", 0));
        balance = j.value("balance", 0.0);
        currency = j.value("currency", "USD");
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- VIRTUAL CARD ---
    VirtualCard::VirtualCard() : Entity(), dailyLimit(0.0), currentDailySpend(0.0), isFrozen(false) {}
    VirtualCard::VirtualCard(std::string accId, std::string holder, double limit)
        : Entity(), accountId(accId), cardHolderName(holder), dailyLimit(limit), currentDailySpend(0.0), isFrozen(false) {
        cardNumber = generateCardNumber();
        cvv = generateCVV();
        expiryDate = generateExpiryDate();
    }

    std::string VirtualCard::generateCardNumber() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 9999);
        return "4000 " + std::to_string(dis(gen)) + " " + std::to_string(dis(gen)) + " " + std::to_string(dis(gen));
    }

    std::string VirtualCard::generateCVV() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(100, 999);
        return std::to_string(dis(gen));
    }

    std::string VirtualCard::generateExpiryDate() {
        return "12/28"; // Mock logic for simplicity
    }

    std::string VirtualCard::getAccountId() const { return accountId; }
    std::string VirtualCard::getCardNumber() const { return cardNumber; }
    double VirtualCard::getDailyLimit() const { return dailyLimit; }
    bool VirtualCard::getIsFrozen() const { return isFrozen; }
    
    void VirtualCard::toggleFreeze() {
        isFrozen = !isFrozen;
        updateTimestamp();
    }

    bool VirtualCard::processPayment(double amount) {
        if (isFrozen) return false;
        if (currentDailySpend + amount > dailyLimit) return false;
        
        currentDailySpend += amount;
        updateTimestamp();
        return true;
    }

    void VirtualCard::resetDailySpend() {
        currentDailySpend = 0;
        updateTimestamp();
    }

    json VirtualCard::toJson() const {
        return {
            {"id", id},
            {"accountId", accountId},
            {"cardNumber", cardNumber},
            {"cardHolderName", cardHolderName},
            {"expiryDate", expiryDate},
            {"cvv", cvv},
            {"dailyLimit", dailyLimit},
            {"currentDailySpend", currentDailySpend},
            {"isFrozen", isFrozen},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void VirtualCard::fromJson(const json& j) {
        id = j.value("id", id);
        accountId = j.value("accountId", "");
        cardNumber = j.value("cardNumber", "");
        cardHolderName = j.value("cardHolderName", "");
        expiryDate = j.value("expiryDate", "");
        cvv = j.value("cvv", "");
        dailyLimit = j.value("dailyLimit", 0.0);
        currentDailySpend = j.value("currentDailySpend", 0.0);
        isFrozen = j.value("isFrozen", false);
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- TRANSACTION ---
    Transaction::Transaction() : Entity(), amount(0.0) {}
    Transaction::Transaction(std::string accId, double amt, TransactionType t, Category cat, std::string desc)
        : Entity(), accountId(accId), amount(amt), type(t), category(cat), description(desc) {}

    std::string Transaction::getAccountId() const { return accountId; }
    double Transaction::getAmount() const { return amount; }
    TransactionType Transaction::getType() const { return type; }
    Category Transaction::getCategory() const { return category; }
    std::string Transaction::getDescription() const { return description; }

    void Transaction::setAmount(double amt) { amount = amt; updateTimestamp(); }
    void Transaction::setCategory(Category cat) { category = cat; updateTimestamp(); }
    void Transaction::setDescription(const std::string& desc) { description = desc; updateTimestamp(); }

    void Transaction::setLinkedCardId(const std::string& cardId) { linkedCardId = cardId; }
    void Transaction::setTargetAccountId(const std::string& tAccId) { targetAccountId = tAccId; }

    json Transaction::toJson() const {
        return {
            {"id", id},
            {"accountId", accountId},
            {"linkedCardId", linkedCardId},
            {"amount", amount},
            {"type", static_cast<int>(type)},
            {"category", static_cast<int>(category)},
            {"description", description},
            {"targetAccountId", targetAccountId},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Transaction::fromJson(const json& j) {
        id = j.value("id", id);
        accountId = j.value("accountId", "");
        linkedCardId = j.value("linkedCardId", "");
        amount = j.value("amount", 0.0);
        type = static_cast<TransactionType>(j.value("type", 0));
        category = static_cast<Category>(j.value("category", 0));
        description = j.value("description", "");
        targetAccountId = j.value("targetAccountId", "");
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- BUDGET ---
    Budget::Budget() : Entity(), limitAmount(0.0), currentSpent(0.0) {}
    Budget::Budget(std::string uId, std::string n, Category cat, double limit, int targetVal, std::string period)
        : Entity(), userId(uId), name(n), category(cat), limitAmount(limit), currentSpent(0.0), targetValue(targetVal), periodType(period) {}

    std::string Budget::getUserId() const { return userId; }
    std::string Budget::getName() const { return name; }
    Category Budget::getCategory() const { return category; }
    double Budget::getLimitAmount() const { return limitAmount; }
    double Budget::getCurrentSpent() const { return currentSpent; }

    void Budget::setName(const std::string& newName) {
        name = newName;
        updateTimestamp();
    }

    void Budget::setLimitAmount(double newLimit) {
        limitAmount = newLimit;
        updateTimestamp();
    }

    void Budget::addExpense(double amount) {
        currentSpent += amount;
        updateTimestamp();
    }

    bool Budget::isExceeded() const {
        return currentSpent > limitAmount;
    }

    double Budget::getRemaining() const {
        return limitAmount - currentSpent;
    }

    json Budget::toJson() const {
        return {
            {"id", id},
            {"userId", userId},
            {"name", name},
            {"category", static_cast<int>(category)},
            {"limitAmount", limitAmount},
            {"currentSpent", currentSpent},
            {"targetValue", targetValue},
            {"periodType", periodType},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Budget::fromJson(const json& j) {
        id = j.value("id", id);
        userId = j.value("userId", "");
        name = j.value("name", "");
        category = static_cast<Category>(j.value("category", 0));
        limitAmount = j.value("limitAmount", 0.0);
        currentSpent = j.value("currentSpent", 0.0);
        targetValue = j.value("targetValue", 0);
        periodType = j.value("periodType", "Monthly");
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- GOAL ---
    Goal::Goal() : Entity(), targetAmount(0.0), currentSaved(0.0) {}
    Goal::Goal(std::string uId, std::string name, double target, std::time_t dead)
        : Entity(), userId(uId), goalName(name), targetAmount(target), currentSaved(0.0), deadline(dead) {}

    void Goal::addFunds(double amount) {
        currentSaved += amount;
        updateTimestamp();
    }

    double Goal::getProgressPercentage() const {
        if (targetAmount == 0) return 0;
        return (currentSaved / targetAmount) * 100.0;
    }

    bool Goal::isAchieved() const {
        return currentSaved >= targetAmount;
    }

    json Goal::toJson() const {
        return {
            {"id", id},
            {"userId", userId},
            {"goalName", goalName},
            {"targetAmount", targetAmount},
            {"currentSaved", currentSaved},
            {"deadline", deadline},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Goal::fromJson(const json& j) {
        id = j.value("id", id);
        userId = j.value("userId", "");
        goalName = j.value("goalName", "");
        targetAmount = j.value("targetAmount", 0.0);
        currentSaved = j.value("currentSaved", 0.0);
        deadline = j.value("deadline", createdAt);
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- SUBSCRIPTION ---
    Subscription::Subscription() : Entity(), costPerBillingCycle(0.0) {}
    Subscription::Subscription(std::string accId, std::string service, double cost, std::time_t nextBill)
        : Entity(), accountId(accId), serviceName(service), costPerBillingCycle(cost), nextBillingDate(nextBill), status(SubscriptionStatus::ACTIVE) {}

    bool Subscription::processBilling(double& outAmount) {
        if (status == SubscriptionStatus::ACTIVE && std::time(nullptr) >= nextBillingDate) {
            outAmount = costPerBillingCycle;
            // Advance by ~30 days
            nextBillingDate += 30 * 24 * 60 * 60;
            updateTimestamp();
            return true;
        }
        return false;
    }

    void Subscription::cancel() {
        status = SubscriptionStatus::CANCELLED;
        updateTimestamp();
    }

    void Subscription::pause() {
        if(status == SubscriptionStatus::ACTIVE) status = SubscriptionStatus::PAUSED;
        updateTimestamp();
    }

    void Subscription::resume() {
        if(status == SubscriptionStatus::PAUSED) status = SubscriptionStatus::ACTIVE;
        updateTimestamp();
    }

    json Subscription::toJson() const {
        return {
            {"id", id},
            {"accountId", accountId},
            {"serviceName", serviceName},
            {"costPerBillingCycle", costPerBillingCycle},
            {"nextBillingDate", nextBillingDate},
            {"status", static_cast<int>(status)},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Subscription::fromJson(const json& j) {
        id = j.value("id", id);
        accountId = j.value("accountId", "");
        serviceName = j.value("serviceName", "");
        costPerBillingCycle = j.value("costPerBillingCycle", 0.0);
        nextBillingDate = j.value("nextBillingDate", createdAt);
        status = static_cast<SubscriptionStatus>(j.value("status", 0));
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

    // --- ALERT ---
    Alert::Alert() : Entity(), isRead(false) {}
    Alert::Alert(std::string uId, std::string msg, AlertType t)
        : Entity(), userId(uId), message(msg), type(t), isRead(false) {}

    std::string Alert::getUserId() const { return userId; }
    std::string Alert::getMessage() const { return message; }
    AlertType Alert::getType() const { return type; }
    bool Alert::getIsRead() const { return isRead; }

    void Alert::markAsRead() {
        isRead = true;
        updateTimestamp();
    }

    json Alert::toJson() const {
        return {
            {"id", id},
            {"userId", userId},
            {"message", message},
            {"type", static_cast<int>(type)},
            {"isRead", isRead},
            {"createdAt", createdAt},
            {"updatedAt", updatedAt}
        };
    }

    void Alert::fromJson(const json& j) {
        id = j.value("id", id);
        userId = j.value("userId", "");
        message = j.value("message", "");
        type = static_cast<AlertType>(j.value("type", 0));
        isRead = j.value("isRead", false);
        createdAt = j.value("createdAt", createdAt);
        updatedAt = j.value("updatedAt", updatedAt);
    }

} // namespace models
