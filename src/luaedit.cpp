#include "luaedit.h"
//#include <stdexcept>
#include <QCursor>
#include <QColor>
#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QPainter>
#include <QTextFormat>
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

    // Setup line number area and line highlight
    line_number_area = new LineNumberArea(this);

    connect(this, &LuaEdit::blockCountChanged, this, &LuaEdit::update_line_number_area_width);
    connect(this, &LuaEdit::updateRequest, this, &LuaEdit::update_line_number_area);
    connect(this, &LuaEdit::cursorPositionChanged, this, &LuaEdit::highlight_current_line);

    update_line_number_area_width(0);
    highlight_current_line();
}

int LuaEdit::line_number_area_width()
{
    if (!m_prefs.value("editor/linenumbers").value<bool>())
        return 0;

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    return (3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits);
}

void LuaEdit::update_line_number_area_width(int /*new_block_count*/)
{
    if (m_prefs.value("editor/linenumbers").value<bool>())
        setViewportMargins(line_number_area_width(), 0, 0, 0);
    else
        setViewportMargins(0, 0, 0, 0);
}

void LuaEdit::update_line_number_area(const QRect &rect, int dy)
{
    if (m_prefs.value("editor/linenumbers").value<bool>())
    {
        if (dy)
            line_number_area->scroll(0, dy);
        else
            line_number_area->update(0, rect.y(), line_number_area->width(), rect.height());
        if (rect.contains(viewport()->rect()))
            update_line_number_area_width(0);
    }
}

void LuaEdit::resizeEvent(QResizeEvent *event)
{
    if (m_prefs.value("editor/linenumbers").value<bool>())
    {
        QPlainTextEdit::resizeEvent(event);
        QRect cr = contentsRect();
        line_number_area->setGeometry(QRect(cr.left(), cr.top(), line_number_area_width(), cr.height()));
    }
}

void LuaEdit::highlight_current_line()
{
    QList<QTextEdit::ExtraSelection> extra_selections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor line_color = QColor(40,40,40);

        selection.format.setBackground(line_color);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extra_selections.append(selection);
    }

    setExtraSelections(extra_selections);
}

void LuaEdit::line_number_area_paint_event(QPaintEvent *event)
{
    if (m_prefs.value("editor/linenumbers").value<bool>())
    {
        QPainter painter(line_number_area);
        painter.fillRect(event->rect(), QColor(40, 40, 40));

        QTextBlock block = firstVisibleBlock();
        int block_number = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid() && top <= event->rect().bottom())
        {
            if (block.isVisible() && bottom >= event->rect().top())
            {
                QString number = QString::number(block_number + 1);
                painter.setPen(Qt::lightGray);
                painter.drawText(0, top, line_number_area->width(), fontMetrics().height(), Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++block_number;
        }
    }
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
