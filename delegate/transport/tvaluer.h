#ifndef TVALUER_H
#define TVALUER_H

// read only

#include <QLocale>
#include <QStyledItemDelegate>

#include "component/enumclass.h"
#include "component/settings.h"
#include "component/using.h"

class TValueR : public QStyledItemDelegate {
public:
    TValueR(CSPCTrans trans, const QHash<Section, const SectionRule*>* section_rule_hash, bool is_value, QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    int GetDecimal(const QModelIndex& index) const;

private:
    CSPCTrans trans_ {};
    const QHash<Section, const SectionRule*>* section_rule_hash_ {};
    bool is_value_ {};
    QLocale locale_ {};
};

#endif // TVALUER_H
