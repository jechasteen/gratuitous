#ifndef PREFS_H
#define PREFS_H

#include <QDebug>

#include <QFont>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonDocument>
#include <QSettings>

class Prefs : public QDialog
{
    Q_OBJECT
public:
    explicit Prefs(QWidget *parent = nullptr);

    // wrappers for QSettings calls
    QFont get_font() const;
    void set_font(QFont font);

private:
    // helpers
    void change_font_display(QFont font);

signals:
    void settings_applied();

public slots:
    void do_save() { hide(); do_apply(); }
    void do_apply();
    void do_cancel() { hide(); }

    void do_select_path();
    void do_select_font();
    void do_font_resize(int size);
private:
    // ui elements
    QGridLayout *m_layout;
    QGroupBox *m_group_general;
    QGroupBox *m_group_font;
    QGroupBox *m_group_editor;
    QGroupBox *m_group_buttons;

    QString get_default_path();
    void set_defaults();
    void setup_ui();

    // font is nested inside editor
    void ui_editor_font();
    void ui_general();
    void ui_editor();
    void ui_buttons();

    QSettings m_prefs;
};

#endif // PREFS_H
