#include "treevalue.h"

#include <QPainter>

#include "component/enumclass.h"

TreeValue::TreeValue(const int* decimal, CStringHash* unit_symbol_hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_symbol_hash_ { unit_symbol_hash }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

void TreeValue::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, FormattedString(index));
}

QSize TreeValue::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QFontMetrics(option.font).horizontalAdvance(FormattedString(index)) + 8, option.rect.height());
}

QString TreeValue::FormattedString(const QModelIndex& index) const
{
    int unit { index.sibling(index.row(), static_cast<int>(NodeColumn::kUnit)).data().toInt() };
    return unit_symbol_hash_->value(unit, QString()) + locale_.toString(index.data().toDouble(), 'f', *decimal_);
}
