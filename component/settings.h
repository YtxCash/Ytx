#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

// Struct representing interface settings
struct Interface {
    QString theme {};
    QString language {};
    QString date_format {};
    QString separator {};

    // Equality operator overload to compare two Interface structs
    bool operator==(const Interface& other) const noexcept
    {
        return std::tie(theme, language, date_format, separator) == std::tie(other.theme, other.language, other.date_format, other.separator);
    }

    // Inequality operator overload to compare two Interface structs
    bool operator!=(const Interface& other) const noexcept { return !(*this == other); }
};

// Struct representing section rules
struct SectionRule {
    QString static_label {};
    int static_node {};
    QString dynamic_label {};
    int dynamic_node_lhs {};
    QString operation {};
    int dynamic_node_rhs {};
    bool hide_time {};
    int base_unit {};
    QString document_dir {};
    int value_decimal {};
    int ratio_decimal {};

    // Equality operator overload to compare two SectionRule structs
    bool operator==(const SectionRule& other) const noexcept
    {
        return std::tie(static_label, static_node, dynamic_label, dynamic_node_lhs, operation, dynamic_node_rhs, hide_time, base_unit, document_dir,
                   value_decimal, ratio_decimal)
            == std::tie(other.static_label, other.static_node, other.dynamic_label, other.dynamic_node_lhs, other.operation, other.dynamic_node_rhs,
                other.hide_time, other.base_unit, other.document_dir, other.value_decimal, other.ratio_decimal);
    }

    // Inequality operator overload to compare two SectionRule structs
    bool operator!=(const SectionRule& other) const noexcept { return !(*this == other); }
};

#endif // SETTINGS_H
