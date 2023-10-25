#ifndef ORIGINALDOCUMENT_H
#define ORIGINALDOCUMENT_H

#include <QDialog>
#include <QStringListModel>

#include "component/enumclass.h"
#include "component/using.h"

namespace Ui {
class OriginalDocument;
}

class OriginalDocument : public QDialog {
    Q_OBJECT

public:
    OriginalDocument(Section section, SPTrans& trans, CString& document_dir, QWidget* parent = nullptr);
    ~OriginalDocument();

private slots:
    void on_pBtnAdd_clicked();
    void on_pBtnRemove_clicked();
    void RCustomAccept();
    void on_listView_doubleClicked(const QModelIndex& index);

private:
    void CreateList(CStringList& list);
    void IniDialog();
    void IniConnect();

private:
    Ui::OriginalDocument* ui;
    Section section {};
    SPTrans trans_ {};
    QStringListModel* list_model_ {};
    QString document_dir_ {};
};

#endif // ORIGINALDOCUMENT_H
