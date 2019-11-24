#include "prefs.h"

#include <QCheckBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStandardPaths>

Prefs::Prefs(QWidget *parent) : QDialog(parent)
{
    if (!m_prefs.contains("init"))
        set_defaults();

    setup_ui();
    setWindowTitle("gratuitous preferences");
    setLayout(m_layout);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
}


QString Prefs::get_default_path()
{
    QString default_path;
    QStringList config_paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    for (const QString &path : config_paths)
    {
        QFileInfo dir(path);
        if (!dir.exists() || !dir.isDir() || !dir.isWritable())
            continue;

        dir.setFile(path + "/awesome");
        if (!dir.exists() || !dir.isDir() || !dir.isWritable())
            default_path = path;
        else
            default_path = path + "/awesome";
        break;
    }
    if (default_path.isEmpty())
        default_path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];

    return default_path;
}

void Prefs::set_defaults()
{
    m_prefs.setValue("init", true);

    // Font
    auto *default_font = new QFont();
    default_font->setFamily("Hack");
    default_font->setFixedPitch(true);
    default_font->setPointSize(10);
    m_prefs.setValue("font/family", default_font->family());
    m_prefs.setValue("font/fixed", default_font->fixedPitch());
    m_prefs.setValue("font/size", default_font->pointSize());

    // Word wrap
    m_prefs.setValue("editor/wordwrap", false);

    // Default path, for file dialog open location
    m_prefs.setValue("path", get_default_path());

    // Whether or not to maximize new windows
    m_prefs.setValue("maximize_new", true);
}

void Prefs::setup_ui()
{
    m_layout = new QGridLayout;

    ui_general();
    ui_editor();
    ui_buttons();

    m_layout->addWidget(m_group_general);
    m_layout->addWidget(m_group_editor);
    m_layout->addWidget(m_group_buttons);
}

void Prefs::ui_general()
{
    m_group_general = new QGroupBox(tr("general"));
    auto *general_layout = new QVBoxLayout;

    // Default Path
    auto path_group = new QGroupBox(tr("default path for dialogs"));
    auto *path_layout = new QHBoxLayout;
    auto *path_text = new QLineEdit(this);
    path_text->setEnabled(false);
    path_text->setText(m_prefs.value("path").value<QString>());
    auto *path_change = new QPushButton(tr("set path"));
    connect(path_change, &QPushButton::pressed, this, &Prefs::do_select_path);

    path_layout->addWidget(path_text);
    path_layout->addWidget(path_change);
    path_group->setLayout(path_layout);

    general_layout->addWidget(path_group);

    m_group_general->setLayout(general_layout);
}

void Prefs::ui_editor_font()
{
    m_group_font = new QGroupBox(tr("font"));
    auto *font_layout = new QHBoxLayout;
    auto *font_family = new QLineEdit(this);
    font_family->setText(m_prefs.value("font/family").value<QString>());
    font_family->setReadOnly(true);
    font_family->setFont(QFont(m_prefs.value("font/family").value<QString>(), m_prefs.value("font/size").value<int>()));
    font_family->setFixedWidth(200);
    auto *font_size = new QSpinBox(this);
    font_size->setRange(0, 100);
    font_size->setValue(m_prefs.value("font/size").value<int>());
    font_size->setSingleStep(1);
    connect(font_size, QOverload<int>::of(&QSpinBox::valueChanged), this, &Prefs::do_font_resize);
    auto *font_change = new QPushButton(tr("select font"));
    connect(font_change, &QPushButton::pressed, this, &Prefs::do_select_font);

    font_layout->addStretch(1);
    font_layout->addWidget(font_family, 1, Qt::AlignTop);
    font_layout->addWidget(font_size, 1, Qt::AlignTop);
    font_layout->addWidget(font_change, 1, Qt::AlignTop);
    m_group_font->setLayout(font_layout);
}

void Prefs::ui_editor()
{
    m_group_editor = new QGroupBox(tr("editor"));
    auto *editor_layout = new QVBoxLayout;

    auto *editor_wordwrap = new QCheckBox(tr("word wrap"), this);
    editor_wordwrap->setChecked(m_prefs.value("editor/wordwrap").value<bool>());
    editor_layout->addWidget(editor_wordwrap);

    ui_editor_font();
    editor_layout->addWidget(m_group_font);
    m_group_editor->setLayout(editor_layout);
}

void Prefs::ui_buttons()
{
    m_group_buttons = new QGroupBox;
    auto *button_layout = new QHBoxLayout(this);
    auto *button_save = new QPushButton(tr("save"), this);
    connect(button_save, &QPushButton::pressed, this, &Prefs::do_save);
    auto *button_apply = new QPushButton(tr("apply"), this);
    connect(button_apply, &QPushButton::pressed, this, &Prefs::do_apply);
    auto *button_cancel = new QPushButton(tr("cancel"), this);
    connect(button_cancel, &QPushButton::pressed, this, &Prefs::do_cancel);

    button_layout->addWidget(button_save);
    button_layout->addWidget(button_apply);
    button_layout->addWidget(button_cancel);
    m_group_buttons->setLayout(button_layout);
}

// Public API
QFont Prefs::get_font() const
{
    QFont current_font;
    current_font.setFamily(m_prefs.value("font/family").value<QString>());
    current_font.setPointSize(m_prefs.value("font/size").value<int>());
    current_font.setFixedPitch(m_prefs.value("font/fixed").value<bool>());
    return current_font;
}

void Prefs::set_font(QFont font)
{
    m_prefs.setValue("font/family", font.family());
    m_prefs.setValue("font/size", font.pointSize());
    m_prefs.setValue("font/fixed", font.fixedPitch());
    change_font_display(font);
}

// Private helpers

void Prefs::change_font_display(QFont font)
{
    m_group_font->findChild<QLineEdit*>()->setText(font.family());
    m_group_font->findChild<QLineEdit*>()->setFont(font);
    m_group_font->findChild<QSpinBox*>()->setValue(font.pointSize());
}

// Slots
void Prefs::do_apply()
{
    set_font(m_group_font->findChild<QLineEdit*>()->font());
    m_prefs.setValue("editor/wordwrap", m_group_editor->findChild<QCheckBox*>()->isChecked());
    emit settings_applied();
}

void Prefs::do_select_path()
{
    QString new_path = QFileDialog::getExistingDirectory(this, tr("set default path"), "/");
    if (!new_path.isEmpty())
    {
        m_group_general->findChild<QLineEdit*>()->setText(new_path);
    }
}

void Prefs::do_select_font()
{
    QFont current_font = get_font();
    bool ok;
    QFont font = QFontDialog::getFont(&ok, current_font, this);
    if (ok)
    {
        auto *new_font = new QFont(font);
        change_font_display(*new_font);
    }
}

void Prefs::do_font_resize(int size)
{
    QFont current_font = get_font();
    current_font.setPointSize(size);
    m_group_font->findChild<QLineEdit*>()->setFont(current_font);
    change_font_display(current_font);
}
