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
    int transport {}; // Nothing = 0, Sender = 1, Receiver = 2
    QStringList location {}; // section; trans_id; section; trans_id;
    QStringList document {};
    bool state { false };
    double rhs_credit {};
    double rhs_debit {};
    double rhs_ratio { 1.0 };
    int rhs_node {};

    Transaction() = default;
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    Transaction& operator=(Transaction&&) = delete;

    void Reset()
    {
        id = 0;
        transport = 0;
        location.clear();
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
    int* transport {};
    QStringList* location {};
    QStringList* document {};
    bool* state {};
    double* related_credit {};
    double* related_debit {};
    double* related_ratio {};
    int* related_node {};

    double balance {};

    Trans() = default;
    Trans(const Trans&) = delete;
    Trans& operator=(const Trans&) = delete;
    Trans(Trans&&) = delete;
    Trans& operator=(Trans&&) = delete;

    void Reset()
    {
        id = nullptr;
        transport = nullptr;
        location = nullptr;
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
