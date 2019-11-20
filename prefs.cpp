#include "prefs.h"

#include <QGroupBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFont>

Prefs::Prefs(QWidget *parent) : QWidget(parent)
{
    m_vlayout = new QVBoxLayout;
    set_defaults();
    m_current = m_default;
    setup_options();
    hide();
    setWindowTitle("gratuitous preferences");
}

void Prefs::set_defaults()
{
    QFont *default_font = new QFont;
    default_font->setFamily("Hack");
    default_font->setFixedPitch(true);
    default_font->setPixelSize(10);
    m_default.font = default_font;
}

void Prefs::setup_options()
{
    QGroupBox *font_group = new QGroupBox(tr("font"));
    font_group->setTitle("font");
    QVBoxLayout *font_layout = new QVBoxLayout;
    font_layout->addStretch(1);
    QLineEdit *font_current = new QLineEdit(this);
    font_current->setText(m_current.font->family() + ", " + QString::number(m_current.font->pixelSize()));
    font_current->setReadOnly(true);

    font_group->setLayout(font_layout);
    font_layout->addWidget(font_current);
    m_vlayout->addWidget(font_group);
}
