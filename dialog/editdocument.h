#ifndef EDITDOCUMENT_H
#define EDITDOCUMENT_H

#include <QDialog>
#include <QStringListModel>

#include "component/enumclass.h"
#include "component/using.h"

namespace Ui {
class EditDocument;
}

class EditDocument final : public QDialog {
    Q_OBJECT

public:
    explicit EditDocument(Section section, SPTrans& trans, CString& document_dir, QWidget* parent = nullptr);
    ~EditDocument();

private slots:
    void on_pBtnAdd_clicked();
    void on_pBtnRemove_clicked();
    void RCustomAccept();
    void on_listView_doubleClicked(const QModelIndex& index);

private:
    void CreateList(CStringList& list);
    void IniConnect();

private:
    Ui::EditDocument* ui;
    Section section {};
    SPTrans trans_ {};
    QStringListModel* list_model_ {};
    QString document_dir_ {};
};

#endif // EDITDOCUMENT_H
