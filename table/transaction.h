#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QStringList>

struct Transaction {
    int id {};
    QString date_time {};
    QString code {};
    int lhs_node {};
    double lhs_ratio { 1.0 };
    double lhs_debit {};
    double lhs_credit {};
    QString description {};
    int rhs_node {};
    double rhs_ratio { 1.0 };
    double rhs_debit {};
    double rhs_credit {};
    bool state { false };
    QStringList document {};

    Transaction() = default;
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(Transaction&&) = delete;

    void Reset()
    {
        id = 0;
        date_time.clear();
        code.clear();
        lhs_node = 0;
        lhs_ratio = 1.0;
        lhs_debit = 0.0;
        lhs_credit = 0.0;
        description.clear();
        rhs_node = 0;
        rhs_ratio = 1.0;
        rhs_debit = 0.0;
        rhs_credit = 0.0;
        state = false;
        document.clear();
    }
};

struct Trans {
    int* id {};
    QString* date_time {};
    QString* code {};
    int* node {};
    double* ratio {};
    double* debit {};
    double* credit {};
    QString* description {};
    int* related_node {};
    double* related_ratio {};
    double* related_debit {};
    double* related_credit {};
    bool* state {};
    QStringList* document {};

    double balance {};

    Trans() = default;
    Trans(const Trans&) = delete;
    Trans& operator=(const Trans&) = delete;
    Trans(Trans&&) = delete;
    Trans& operator=(Trans&&) = delete;

    void Reset()
    {
        id = nullptr;
        date_time = nullptr;
        code = nullptr;
        node = nullptr;
        ratio = nullptr;
        debit = nullptr;
        credit = nullptr;
        description = nullptr;
        related_node = nullptr;
        related_ratio = nullptr;
        related_debit = nullptr;
        related_credit = nullptr;
        state = nullptr;
        document = nullptr;

        balance = 0.0;
    }
};

#endif // TRANSACTION_H
