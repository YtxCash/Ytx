#ifndef EDITNODEFIANNCE_H
#define EDITNODEFIANNCE_H

#include <QDialog>

#include "component/info.h"
#include "component/using.h"

namespace Ui {
class EditNodeFinance;
}

class EditNodeFinance final : public QDialog {
    Q_OBJECT

public:
    EditNodeFinance(Node* node, CString* separator, const Info* info, bool node_usage, bool view_opened, CString& parent_path, QWidget* parent = nullptr);
    ~EditNodeFinance();

private slots:
    void REditName(const QString& arg1);

    void on_lineEditName_editingFinished();
    void on_lineEditCode_editingFinished();
    void on_lineEditDescription_editingFinished();
    void on_comboUnit_currentIndexChanged(int index);
    void on_rBtnDDCI_toggled(bool checked);
    void on_chkBoxBranch_toggled(bool checked);
    void on_plainTextEdit_textChanged();

protected:
    void changeEvent(QEvent* event) override;

private:
    void IniDialog(CStringHash* currency_map);
    void IniConnect();
    void Data(Node* node);

private:
    Ui::EditNodeFinance* ui;
    Node* node_ {};
    CString* separator_ {};

    bool node_usage_ {};
    bool view_opened_ {};
    QStringList node_name_list_ {};
    QString parent_path_ {};
};

#endif // EDITNODEFIANNCE_H
