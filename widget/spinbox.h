#ifndef SPINBOX_H
#define SPINBOX_H

#include <QSpinBox>

class SpinBox final : public QSpinBox {
    Q_OBJECT

public:
    SpinBox(int min, int max, QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
};

#endif // SPINBOX_H
