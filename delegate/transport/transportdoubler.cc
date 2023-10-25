#include "transportdoubler.h"

#include <QPainter>

TransportDoubleR::TransportDoubleR(CSPCTrans trans, const QHash<Section, const SectionRule*>* section_rule_hash, bool is_value, QObject* parent)
    : QStyledItemDelegate { parent }
    , trans_ { trans }
    , section_rule_hash_ { section_rule_hash }
    , is_value_ { is_value }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

void TransportDoubleR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    if (value == 0)
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, locale_.toString(value, 'f', GetDecimal(index)));
}

QSize TransportDoubleR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    return value == 0 ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(locale_.toString(value, 'f', GetDecimal(index))) + 8, option.rect.height());
}

int TransportDoubleR::GetDecimal(const QModelIndex& index) const
{
    auto section_rule { section_rule_hash_->value(Section { trans_->location->at(index.row() * 2).toInt() }) };
    return is_value_ ? section_rule->value_decimal : section_rule->ratio_decimal;
}
