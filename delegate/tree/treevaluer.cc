#include "treevaluer.h"

#include <QPainter>

#include "component/enumclass.h"

TreeValueR::TreeValueR(const int* decimal, CStringHash* unit_symbol_hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , unit_symbol_hash_ { unit_symbol_hash }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

void TreeValueR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, FormattedString(index));
}

QSize TreeValueR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(QFontMetrics(option.font).horizontalAdvance(FormattedString(index)) + 8, option.rect.height());
}

QString TreeValueR::FormattedString(const QModelIndex& index) const
{
    int unit { index.sibling(index.row(), std::to_underlying(TreeColumn::kUnit)).data().toInt() };
    return unit_symbol_hash_->value(unit, QString()) + locale_.toString(index.data().toDouble(), 'f', *decimal_);
}
