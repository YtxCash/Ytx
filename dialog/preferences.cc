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
{
    ui->setupUi(this);
    IniStringList();
    IniDialog(&info->unit_hash, date_format_list);
    IniConnect();

    Data();
    RenameLable(info->section);
}

Preferences::~Preferences() { delete ui; }

void Preferences::IniDialog(CStringHash* unit_hash, CStringList* date_format_list)
{
    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBtnOk->setDefault(true);
    this->setWindowTitle(tr("Preferences"));

    IniCombo(ui->comboDateTime, *date_format_list);
    IniCombo(ui->comboLanguage, language_list_);
    IniCombo(ui->comboSeparator, separator_list_);
    IniCombo(ui->comboTheme, theme_list_);

    IniCombo(ui->comboBaseUnit, unit_hash);

    IniCombo(ui->comboDynamicLhs, leaf_path_, branch_path_);
    IniCombo(ui->comboStatic, leaf_path_, branch_path_);
    IniCombo(ui->comboOperation, operation_list_);
    IniCombo(ui->comboDynamicRhs, leaf_path_, branch_path_);
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

void Preferences::IniCombo(QComboBox* combo, CStringHash* hash_1st, CStringHash* hash_2nd)
{
    IniCombo(combo);

    combo->blockSignals(true);
    for (auto it = hash_1st->cbegin(); it != hash_1st->cend(); ++it)
        combo->addItem(it.value(), it.key());

    if (hash_2nd)
        for (auto it = hash_2nd->cbegin(); it != hash_2nd->cend(); ++it)
            combo->addItem(it.value(), it.key());

    combo->model()->sort(0);
    combo->blockSignals(false);
}

void Preferences::IniCombo(QComboBox* combo, CStringList& list)
{
    IniCombo(combo);

    combo->blockSignals(true);
    combo->addItems(list);
    combo->model()->sort(0);
    combo->blockSignals(false);
}

void Preferences::IniConnect()
{
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Preferences::on_pBtnApply_clicked);
}

void Preferences::Data()
{
    DataCombo(ui->comboTheme, interface_.theme);
    DataCombo(ui->comboLanguage, interface_.language);
    DataCombo(ui->comboSeparator, interface_.separator);
    DataCombo(ui->comboDateTime, interface_.date_format);

    DataCombo(ui->comboBaseUnit, section_rule_.base_unit);
    ui->pBtnDocumentDir->setText(section_rule_.document_dir);
    ui->spinValueDecimal->setValue(section_rule_.value_decimal);
    ui->spinRatioDecimal->setValue(section_rule_.ratio_decimal);

    ui->lineStatic->setText(section_rule_.static_label);
    DataCombo(ui->comboStatic, section_rule_.static_node);
    ui->lineDynamic->setText(section_rule_.dynamic_label);
    DataCombo(ui->comboDynamicLhs, section_rule_.dynamic_node_lhs);
    DataCombo(ui->comboOperation, section_rule_.operation);
    DataCombo(ui->comboDynamicRhs, section_rule_.dynamic_node_rhs);

    ui->comboDynamicLhs->insertItem(0, QString(), 0);
    ui->comboDynamicRhs->insertItem(0, QString(), 0);
    ui->comboStatic->insertItem(0, QString(), 0);

    ui->checkHideTime->setChecked(section_rule_.hide_time);

    ResizeLine(ui->lineStatic, section_rule_.static_label);
    ResizeLine(ui->lineDynamic, section_rule_.dynamic_label);
}

void Preferences::DataCombo(QComboBox* combo, int value)
{
    combo->blockSignals(true);
    int item_index { combo->findData(value) };
    combo->setCurrentIndex(item_index);
    combo->blockSignals(false);
}

void Preferences::DataCombo(QComboBox* combo, CString& string)
{
    combo->blockSignals(true);
    int item_index { combo->findText(string) };
    combo->setCurrentIndex(item_index);
    combo->blockSignals(false);
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

void Preferences::on_pBtnApply_clicked() { emit SUpdateSettings(section_rule_, interface_); }

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

void Preferences::ResizeLine(QLineEdit* line, CString& text) { line->setMinimumWidth(QFontMetrics(line->font()).horizontalAdvance(text) + 8); }

void Preferences::RenameLable(Section section)
{
    switch (section) {
    case Section::kFinance:
        ui->labelBaseUnit->setText(tr("Base Currency"));
        ui->labelRatioDecimal->setText(tr("FXRate Decimal"));
        break;
    case Section::kStakeholder:
        ui->labelRatioDecimal->setText(tr("TaxRate Decimal"));
        ui->labelBaseUnit->setText(tr("Default Mark"));
        break;
    case Section::kTask:
    case Section::kProduct:
        ui->labelValueDecimal->setText(tr("Amount Decimal"));
        ui->labelRatioDecimal->setText(tr("Price Decimal"));
        break;
    default:
        break;
    }
}

void Preferences::on_checkHideTime_toggled(bool checked) { section_rule_.hide_time = checked; }

void Preferences::on_comboBaseUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_rule_.base_unit = ui->comboBaseUnit->currentData().toInt();
}

void Preferences::on_comboStatic_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_rule_.static_node = ui->comboStatic->currentData().toInt();
}

void Preferences::on_comboDynamicLhs_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_rule_.dynamic_node_lhs = ui->comboDynamicLhs->currentData().toInt();
}

void Preferences::on_comboDynamicRhs_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_rule_.dynamic_node_rhs = ui->comboDynamicRhs->currentData().toInt();
}

void Preferences::on_spinValueDecimal_editingFinished() { section_rule_.value_decimal = ui->spinValueDecimal->value(); }

void Preferences::on_lineStatic_editingFinished()
{
    section_rule_.static_label = ui->lineStatic->text();
    ResizeLine(ui->lineStatic, section_rule_.static_label);
}

void Preferences::on_lineDynamic_editingFinished()
{
    section_rule_.dynamic_label = ui->lineDynamic->text();
    ResizeLine(ui->lineDynamic, section_rule_.dynamic_label);
}

void Preferences::on_spinRatioDecimal_editingFinished() { section_rule_.ratio_decimal = ui->spinRatioDecimal->value(); }

void Preferences::on_comboTheme_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    interface_.theme = ui->comboTheme->currentText();
}

void Preferences::on_comboLanguage_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    interface_.language = ui->comboLanguage->currentText();
}

void Preferences::on_comboDateTime_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    interface_.date_format = ui->comboDateTime->currentText();
}

void Preferences::on_comboSeparator_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    interface_.separator = ui->comboSeparator->currentText();
}

void Preferences::on_comboOperation_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    section_rule_.operation = ui->comboOperation->currentText();
}
