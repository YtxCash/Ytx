#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QComboBox>
#include <QDialog>

#include "component/enumclass.h"
#include "component/info.h"
#include "component/settings.h"
#include "component/using.h"

namespace Ui {
class Preferences;
}

class Preferences final : public QDialog {
    Q_OBJECT

public:
    Preferences(const Info* info, CStringHash* leaf_path, CStringHash* branch_path, CStringList* date_format_list, const Interface& interface,
        const SectionRule& section_rule, QWidget* parent = nullptr);
    ~Preferences();

signals:
    void SUpdateSettings(const SectionRule& section_rule, const Interface& interface);

private slots:
    void on_pBtnApply_clicked();
    void on_pBtnDocumentDir_clicked();
    void on_pBtnResetDocumentDir_clicked();

    void on_comboBaseUnit_currentIndexChanged(int index);

    void on_comboOperation_currentIndexChanged(int index);
    void on_comboStatic_currentIndexChanged(int index);
    void on_comboDynamicLhs_currentIndexChanged(int index);
    void on_comboDynamicRhs_currentIndexChanged(int index);

    void on_spinValueDecimal_editingFinished();
    void on_lineStatic_editingFinished();
    void on_lineDynamic_editingFinished();
    void on_spinRatioDecimal_editingFinished();

    void on_comboTheme_currentIndexChanged(int index);
    void on_comboLanguage_currentIndexChanged(int index);
    void on_comboDateTime_currentIndexChanged(int index);
    void on_comboSeparator_currentIndexChanged(int index);
    void on_checkHideTime_toggled(bool checked);

private:
    void IniDialog(CStringHash* unit_hash, CStringList* date_format_list);
    void IniCombo(QComboBox* combo);
    void IniCombo(QComboBox* combo, CStringHash* hash_1st, CStringHash* hash_2nd = nullptr);
    void IniCombo(QComboBox* combo, CStringList& list);

    void IniConnect();
    void IniStringList();
    void ResizeLine(QLineEdit* line, CString& text);
    void RenameLable(Section section);

    void Data();
    void DataCombo(QComboBox* combo, int value);
    void DataCombo(QComboBox* combo, CString& string);

private:
    Ui::Preferences* ui;

    QStringList theme_list_ {};
    QStringList language_list_ {};
    QStringList separator_list_ {};
    QStringList operation_list_ {};

    CStringHash* leaf_path_ {};
    CStringHash* branch_path_ {};

    Interface interface_ {};
    SectionRule section_rule_ {};
};

#endif // PREFERENCES_H
