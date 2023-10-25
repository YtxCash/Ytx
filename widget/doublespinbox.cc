#include "doublespinbox.h"

#include <QKeyEvent>

DoubleSpinBox::DoubleSpinBox(const int* decimal, double min, double max, QWidget* parent)
    : QDoubleSpinBox { parent }
{
    this->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->setGroupSeparatorShown(true);
    this->setDecimals(*decimal);
    this->setRange(min, max);
}

void DoubleSpinBox::wheelEvent(QWheelEvent* event) { event->ignore(); }
