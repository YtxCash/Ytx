#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H

#include <QDoubleSpinBox>

class DoubleSpinBox final : public QDoubleSpinBox {
    Q_OBJECT

public:
    DoubleSpinBox(const int* decimal, double min, double max, QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
};

#endif // DOUBLESPINBOX_H
