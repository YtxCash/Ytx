#include "node.h"

Node::~Node()
{
    parent = nullptr;
    children.clear();
}

bool Node::operator==(const Node& other) const noexcept
{
    return std::tie(name, id, code, first_property, second_property, fifth_property, sixth_property, seventh_property, date_time, description, note, node_rule,
               branch, unit, parent, children)
        == std::tie(other.name, other.id, other.code, other.first_property, other.second_property, other.fifth_property, other.sixth_property,
            other.seventh_property, other.date_time, other.description, other.note, other.node_rule, other.branch, other.unit, other.parent, other.children)
        && std::abs(third_property - other.third_property) < tolerance && std::abs(fourth_property - other.fourth_property) < tolerance;
}

bool Node::operator!=(const Node& other) const noexcept { return !(*this == other); }

void Node::Reset()
{
    name.clear();
    id = 0;
    code.clear();
    third_property = 0.0;
    fourth_property = 0.0;
    first_property = 0;
    second_property = 0;
    fifth_property = false;
    sixth_property = false;
    date_time.clear();
    description.clear();
    note.clear();
    node_rule = false;
    branch = false;
    unit = 0;

    final_total = 0.0;
    initial_total = 0.0;

    parent = nullptr;
    children.clear();
}
