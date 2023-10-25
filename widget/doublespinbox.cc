#include "doublespinbox.h"

#include <QKeyEvent>

#include "component/constvalue.h"

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox { parent }
{
    this->setRange(MIN, MAX);
    this->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->setGroupSeparatorShown(true);
}

void DoubleSpinBox::wheelEvent(QWheelEvent* event) { event->ignore(); }
