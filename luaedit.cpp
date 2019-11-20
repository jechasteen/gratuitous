#include "luaedit.h"

#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QTextStream>

LuaEdit::LuaEdit(QWidget *parent, QString filename)
{
    m_parent = parent;
    m_filename = std::move(filename);
    m_highlighter = new Highlighter(document());
    if (!m_filename.isEmpty())
        load_from_file();

    setup_appearance();
}

void LuaEdit::load_from_file()
{
    if (!m_filename.isEmpty())
    {
        QFile file(m_filename);
        if (file.open(QIODevice::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            QString text = in.readAll();
            setText(text);
        }
        else
        {
            QMessageBox::warning(this, "Warning", "Failed to open file " + m_filename + ": " + file.errorString());
            return;
        }
    }
    else
        qDebug() << "A LuaEdit widget attempted to load a file, but m_filename was empty.";
}

void LuaEdit::setup_appearance()
{
    QFont m_font("Hack");
    m_font.setFixedPitch(true);
    m_font.setPixelSize(10);
    setFont(m_font);

    setWordWrapMode(QTextOption::NoWrap);
}
