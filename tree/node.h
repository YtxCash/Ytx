#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>

struct Node {
    Node() = default;
    ~Node();
    Node(const Node& other) = default;

    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;

    static constexpr double tolerance = 1e-9;
    bool operator==(const Node& other) const noexcept;
    bool operator!=(const Node& other) const noexcept;

    void Reset();

    QString name {};
    int id {};
    QString code {};
    int first_property {};
    int second_property {};
    double third_property {};
    double fourth_property {};
    bool fifth_property {};
    bool sixth_property {};
    int seventh_property {};
    QString date_time {};
    QString description {};
    QString note {};
    bool node_rule { false };
    bool branch { false };
    int unit {};

    double final_total {};
    double initial_total {};

    Node* parent {};
    QList<Node*> children {};
};

#endif // NODE_H
