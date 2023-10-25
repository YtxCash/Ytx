#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QString>

struct Node {
    QString name {};
    int id {};
    QString code {};
    QString description {};
    QString note {};
    bool node_rule { false };
    bool branch { false };
    int unit {};

    double base_total {};
    double foreign_total {};

    Node* parent {};
    QList<Node*> children {};

    Node() = default;

    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;

    Node(const Node& other)
    {
        this->name = other.name;
        this->id = other.id;
        this->code = other.code;
        this->description = other.description;
        this->note = other.note;
        this->node_rule = other.node_rule;
        this->branch = other.branch;
        this->unit = other.unit;
        this->parent = other.parent;
        this->children = other.children;
        this->base_total = other.base_total;
        this->foreign_total = other.foreign_total;
    };

    ~Node()
    {
        parent = nullptr;
        children.clear();
    };

    bool operator==(const Node& other) const
    {
        return name == other.name && id == other.id && code == other.code && description == other.description && note == other.note
            && node_rule == other.node_rule && branch == other.branch && unit == other.unit && parent == other.parent && children == other.children;
    }

    bool operator!=(const Node& other) const
    {
        return name != other.name || id != other.id || code != other.code || description != other.description || note != other.note
            || node_rule != other.node_rule || branch != other.branch || unit != other.unit || parent != other.parent || children != other.children;
    }

    void Reset()
    {
        name.clear();
        id = 0;
        code.clear();
        description.clear();
        note.clear();
        node_rule = false;
        branch = false;
        unit = 0;

        base_total = 0.0;
        foreign_total = 0.0;

        parent = nullptr;
        children.clear();
    }
};

#endif // NODE_H
