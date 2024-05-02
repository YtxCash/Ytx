#include "preferences.h"

#include <QCompleter>
#include <QDir>
#include <QFileDialog>

#include "component/constvalue.h"
#include "ui_preferences.h"

Preferences::Preferences(const Info* info, CStringHash* leaf_path, CStringHash* branch_path, CStringList* date_format_list, const Interface& interface,
    const SectionRule& section_rule, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Preferences)
    , leaf_path_ { leaf_path }
    , branch_path_ { branch_path }
    , interface_ { interface }
    , section_rule_ { section_rule }
    , separator_ { interface.separator }
{
    ui->setupUi(this);
    IniDialog();
    IniConnect();
    IniStringList();

    SetData(&info->unit_hash, date_format_list);
    RenameLable(info->section);
}

Preferences::~Preferences() { delete ui; }

void Preferences::IniDialog()
{
    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnOk->setDefault(true);
    this->setWindowTitle(tr("Preferences"));

    auto mainwindow_size { qApp->activeWindow()->size() };
    int width { mainwindow_size.width() * 960 / 1920 };
    int height { mainwindow_size.height() * 720 / 1080 };
    this->resize(width, height);

    IniCombo(ui->comboBaseUnit);
    IniCombo(ui->comboDateTime);
    IniCombo(ui->comboLanguage);
    IniCombo(ui->comboSeparator);
    IniCombo(ui->comboTheme);
    IniCombo(ui->comboDynamicLhs);
    IniCombo(ui->comboStatic);
    IniCombo(ui->comboOperation);
    IniCombo(ui->comboDynamicRhs);

    ui->spinRatioDecimal->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->spinValueDecimal->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

void Preferences::IniCombo(QComboBox* combo)
{
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);
}

void Preferences::IniConnect()
{
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex, Qt::UniqueConnection);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Preferences::RCustomAccept, Qt::UniqueConnection);
}

void Preferences::SetData(CStringHash* unit_hash, CStringList* date_format_list)
{
    SetCombo(ui->comboTheme, theme_list_, interface_.theme);
    SetCombo(ui->comboLanguage, language_list_, interface_.language);
    SetCombo(ui->comboSeparator, separator_list_, interface_.separator);
    SetCombo(ui->comboDateTime, *date_format_list, interface_.date_format);

    SetCombo(ui->comboBaseUnit, unit_hash, nullptr, section_rule_.base_unit);
    ui->pBtnDocumentDir->setText(section_rule_.document_dir);
    ui->spinValueDecimal->setValue(section_rule_.value_decimal);
    ui->spinRatioDecimal->setValue(section_rule_.ratio_decimal);

    ui->lineStatic->setText(section_rule_.static_label);
    SetCombo(ui->comboStatic, leaf_path_, branch_path_, section_rule_.static_node);
    ui->lineDynamic->setText(section_rule_.dynamic_label);
    SetCombo(ui->comboDynamicLhs, leaf_path_, branch_path_, section_rule_.dynamic_node_lhs);
    SetCombo(ui->comboOperation, operation_list_, section_rule_.operation);
    SetCombo(ui->comboDynamicRhs, leaf_path_, branch_path_, section_rule_.dynamic_node_rhs);

    ui->checkHideTime->setChecked(section_rule_.hide_time);

    SetLine(ui->lineStatic, section_rule_.static_label);
    SetLine(ui->lineDynamic, section_rule_.dynamic_label);
}

void Preferences::SetCombo(QComboBox* combo, CStringHash* hash_1st, CStringHash* hash_2nd, int value)
{
    for (auto it = hash_1st->cbegin(); it != hash_1st->cend(); ++it)
        combo->addItem(it.value(), it.key());

    if (hash_2nd)
        for (auto it = hash_2nd->cbegin(); it != hash_2nd->cend(); ++it)
            combo->addItem(it.value(), it.key());

    SetIndex(combo, value);
}

void Preferences::SetCombo(QComboBox* combo, CStringList& list, CString& string)
{
    combo->addItems(list);
    combo->model()->sort(0);

    int item_index { combo->findText(string) };
    if (item_index != -1)
        combo->setCurrentIndex(item_index);
}

void Preferences::SetIndex(QComboBox* combo, const QVariant& value)
{
    combo->model()->sort(0);

    int item_index { combo->findData(value) };
    if (item_index != -1)
        combo->setCurrentIndex(item_index);
}

void Preferences::IniStringList()
{
    language_list_.emplaceBack(EN_US);
    language_list_.emplaceBack(ZH_CN);

    separator_list_.emplaceBack(DASH);
    separator_list_.emplaceBack(COLON);
    separator_list_.emplaceBack(SLASH);

    theme_list_.emplaceBack(SOLARIZED_DARK);

    operation_list_.emplaceBack(PLUS);
    operation_list_.emplaceBack(MINUS);
}

void Preferences::on_pBtnApply_clicked()
{
    emit SUpdateSettings(section_rule_, interface_);

    if (separator_ != ui->comboSeparator->currentText()) {
        ui->comboStatic->clear();
        ui->comboDynamicLhs->clear();
        ui->comboDynamicRhs->clear();

        SetCombo(ui->comboStatic, leaf_path_, branch_path_, section_rule_.static_node);
        SetCombo(ui->comboDynamicLhs, leaf_path_, branch_path_, section_rule_.dynamic_node_lhs);
        SetCombo(ui->comboDynamicRhs, leaf_path_, branch_path_, section_rule_.dynamic_node_rhs);

        separator_ = ui->comboSeparator->currentText();
    }
}

void Preferences::on_comboDateTime_textActivated(const QString& arg1) { interface_.date_format = arg1; }

void Preferences::on_comboSeparator_textActivated(const QString& arg1) { interface_.separator = arg1; }

void Preferences::on_spinValueDecimal_valueChanged(int arg1) { section_rule_.value_decimal = arg1; }

void Preferences::on_spinRatioDecimal_valueChanged(int arg1) { section_rule_.ratio_decimal = arg1; }

void Preferences::on_comboTheme_textActivated(const QString& arg1) { interface_.theme = arg1; }

void Preferences::on_comboLanguage_textActivated(const QString& arg1) { interface_.language = arg1; }

void Preferences::RCustomAccept()
{
    emit SUpdateSettings(section_rule_, interface_);
    accept();
}

void Preferences::on_pBtnDocumentDir_clicked()
{
    auto dir { ui->pBtnDocumentDir->text() };
    auto default_dir { QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath() + "/" + dir) };

    if (!default_dir.isEmpty()) {
        auto relative_path { QDir::home().relativeFilePath(default_dir) };
        section_rule_.document_dir = relative_path;
        ui->pBtnDocumentDir->setText(relative_path);
    }
}

void Preferences::on_pBtnResetDocumentDir_clicked()
{
    section_rule_.document_dir = QString();
    ui->pBtnDocumentDir->setText(QString());
}

void Preferences::on_lineStatic_textChanged(const QString& arg1)
{
    section_rule_.static_label = arg1;
    SetLine(ui->lineStatic, arg1);
}

void Preferences::on_lineDynamic_textChanged(const QString& arg1)
{
    section_rule_.dynamic_label = arg1;
    SetLine(ui->lineDynamic, arg1);
}

void Preferences::SetLine(QLineEdit* line, CString& text) { line->setMinimumWidth(QFontMetrics(line->font()).horizontalAdvance(text) + 8); }

void Preferences::RenameLable(Section section)
{
    switch (section) {
    case Section::kFinance:
        ui->label->setText(tr("Base Currency"));
        ui->label_9->setText(tr("FXRate Decimal"));
        break;
    case Section::kNetwork:
        ui->page_2->hide();
        ui->listWidget->setRowHidden(1, true);
        ui->groupBox_2->hide();
        break;
    case Section::kTask:
    case Section::kProduct:
        ui->label_8->setText(tr("Amount Decimal"));
        ui->label_9->setText(tr("Price Decimal"));
        break;
    default:
        break;
    }
}

void Preferences::on_comboStatic_activated(int index) { section_rule_.static_node = ui->comboStatic->itemData(index).toInt(); }

void Preferences::on_comboDynamicLhs_activated(int index) { section_rule_.dynamic_node_lhs = ui->comboDynamicLhs->itemData(index).toInt(); }

void Preferences::on_comboOperation_textActivated(const QString& arg1) { section_rule_.operation = arg1; }

void Preferences::on_comboDynamicRhs_activated(int index) { section_rule_.dynamic_node_rhs = ui->comboDynamicRhs->itemData(index).toInt(); }

void Preferences::on_checkHideTime_toggled(bool checked) { section_rule_.hide_time = checked; }

void Preferences::on_comboBaseUnit_activated(int index) { section_rule_.base_unit = ui->comboBaseUnit->itemData(index).toInt(); }
