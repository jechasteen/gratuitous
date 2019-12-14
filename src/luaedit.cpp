#include "luaedit.h"
#include <stdexcept>

#include <QCursor>
#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QTextStream>

LuaEdit::LuaEdit(QWidget *parent, QString filename)
    : QPlainTextEdit(parent)
{
    m_filename = std::move(filename);
    m_highlighter = new Highlighter(document());
    bool res = false;
    if (!m_filename.isEmpty() && m_filename != GEMPTYFILE)
        res = load_from_file();
    else if (m_filename == GEMPTYFILE)
        return;
    if (!res)
        throw QString("failed to open file");
}

bool LuaEdit::load_from_file()
{
    if (!m_filename.isEmpty())
    {
        QFile file(m_filename);
        if (file.open(QIODevice::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            QString text = in.readAll();
            document()->setPlainText(text);
            //setText(text);
            return true;
        }
        else
        {
            QMessageBox::warning(this, "Warning", "Failed to open file " + m_filename + ": " + file.errorString());
            return false;
        }
    }
    return false;
}
