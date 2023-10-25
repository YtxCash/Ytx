#include "about.h"

#include <QDesktopServices>

#include "ui_about.h"

About::About(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::About)
{
    ui->setupUi(this);
    IniDialog();
}

About::~About() { delete ui; }

void About::IniDialog()
{
    ui->plainTextEdit->setPlainText(tr("Do not go gentle into that good night \n"
                                       "Old age should burn and rave at close of day \n"
                                       "Rage, rage against the dying of the light \n"
                                       "\n"
                                       "Though wise men at their end know dark is right \n"
                                       "Because their words had forked no lightning they \n"
                                       "Do not go gentle into that good night \n"
                                       "\n"
                                       "Good men, the last wave by, crying how bright \n"
                                       "Their frail deeds might have danced in a green bay \n"
                                       "Rage, rage against the dying of the light \n"
                                       "\n"
                                       "Wild men who caught and sang the sun in flight \n"
                                       "And learn, too late, they grieved it on its way \n"
                                       "Do not go gentle into that good night \n"
                                       "\n"
                                       "Grave men, near death, who see with blinding sight \n"
                                       "Blind eyes could blaze like meteors and be gay \n"
                                       "Rage, rage against the dying of the light \n"
                                       "\n"
                                       "And you, my father, there on the sad height \n"
                                       "Curse, bless, me now with your fierce tears, I pray \n"
                                       "Do not go gentle into that good night \n"
                                       "Rage, rage against the dying of the light."));
}

void About::on_pBtnLink_clicked()
{
    auto link { ui->pBtnLink->text() };
    QDesktopServices::openUrl(QUrl(link, QUrl::TolerantMode));
}
