#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit ComboBox(QWidget* parent = nullptr);
};

#endif // COMBOBOX_H
