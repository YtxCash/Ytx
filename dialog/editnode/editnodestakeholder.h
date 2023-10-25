#ifndef EDITNODESTAKEHOLDER_H
#define EDITNODESTAKEHOLDER_H

#include <QDialog>

#include "component/info.h"
#include "component/settings.h"
#include "component/using.h"
#include "sql/sql.h"

namespace Ui {
class EditNodeStakeholder;
}

class EditNodeStakeholder final : public QDialog {
    Q_OBJECT

public:
    EditNodeStakeholder(Node* node, const SectionRule* section_rule, CString* separator, const Info* info, QSharedPointer<Sql> sql, bool view_opened,
        CString& parent_path, CStringHash* node_term_hash, QWidget* parent = nullptr);
    ~EditNodeStakeholder();

private slots:
    void REditName(const QString& arg1);

    void on_lineEditName_editingFinished();
    void on_lineEditCode_editingFinished();
    void on_lineEditDescription_editingFinished();
    void on_chkBoxBranch_toggled(bool checked);
    void on_plainTextEdit_textChanged();
    void on_comboMark_currentIndexChanged(int index);
    void on_comboTerm_currentIndexChanged(int index);
    void on_spinBoxPaymentPeriod_editingFinished();
    void on_dateEditDeadline_editingFinished();
    void on_dSpinBoxTaxRate_editingFinished();
    void on_spinBoxDecimal_editingFinished();

protected:
    void changeEvent(QEvent* event) override;

private:
    void IniDialog(CStringHash* currency_map);
    void IniConnect();
    void Data(Node* node);
    void DisableComponent(bool enable);

private:
    Ui::EditNodeStakeholder* ui;
    Node* node_ {};
    CString* separator_ {};
    CStringHash* node_term_hash_ {};
    const SectionRule* section_rule_ {};

    QSharedPointer<Sql> sql_ {};
    bool view_opened_ {};
    QStringList node_name_list_ {};
    QString parent_path_ {};
};

#endif // EDITNODESTAKEHOLDER_H
