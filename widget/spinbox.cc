#include "spinbox.h"

#include <QWheelEvent>

SpinBox::SpinBox(int min, int max, QWidget* parent)
    : QSpinBox { parent }
{
    this->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    this->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    this->setGroupSeparatorShown(true);
    this->setRange(min, max);
}

void SpinBox::wheelEvent(QWheelEvent* event) { event->ignore(); }
