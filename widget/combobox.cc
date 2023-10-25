#include "combobox.h"

#include <QCompleter>

ComboBox::ComboBox(QWidget* parent)
    : QComboBox { parent }
{
    this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this->setFrame(false);
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(this->model(), this) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    this->setCompleter(completer);
}
