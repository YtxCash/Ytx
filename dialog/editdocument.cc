#include "editdocument.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include "ui_editdocument.h"

EditDocument::EditDocument(Section section, SPTrans& trans, CString& document_dir, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditDocument)
    , section { section }
    , trans_ { trans }
{
    ui->setupUi(this);
    CreateList(*trans->document);
    IniConnect();
    document_dir_ = QDir::homePath() + "/" + document_dir;
}

EditDocument::~EditDocument()
{
    delete list_model_;
    delete ui;
}

void EditDocument::on_pBtnAdd_clicked()
{
    QString filter("*.*");
    auto local_documents { QFileDialog::getOpenFileNames(this, tr("Select Document"), document_dir_, filter, nullptr) };

    if (local_documents.isEmpty())
        return;

    auto existing_documents { trans_->document };
    int row {};

    for (const QString& document : local_documents) {
        if (!existing_documents->contains(document)) {
            row = list_model_->rowCount();
            list_model_->insertRow(row);
            list_model_->setData(list_model_->index(row), QDir::home().relativeFilePath(document));
        }
    }
}

void EditDocument::on_pBtnRemove_clicked()
{
    auto index { ui->listView->currentIndex() };
    list_model_->removeRow(index.row(), QModelIndex());
}

void EditDocument::RCustomAccept() { *trans_->document = list_model_->stringList(); }

void EditDocument::on_listView_doubleClicked(const QModelIndex& index)
{
    QString file_path { QDir::homePath() + "/" + index.data().toString() };
    auto file_url { QUrl::fromLocalFile(file_path) };

    if (QFile::exists(file_path)) {
        QDesktopServices::openUrl(file_url);
        return;
    }

    QMessageBox msg {};
    msg.setIcon(QMessageBox::Critical);
    msg.setText(tr("Document Not Found"));
    msg.setInformativeText(tr("Sorry about that, it seems like the document isn't there. Can you give it another look?"));
    msg.exec();
}

void EditDocument::CreateList(CStringList& list)
{
    list_model_ = new QStringListModel(this);
    list_model_->setStringList(list);
    ui->listView->setModel(list_model_);
}

void EditDocument::IniConnect() { connect(ui->pBtnOk, &QPushButton::clicked, this, &EditDocument::RCustomAccept); }
