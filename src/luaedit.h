#ifndef LUAEDIT_H
#define LUAEDIT_H

#include "highlighter.h"

#include <utility>

#include <QAction>
#include <QDebug>
#include <QSettings>
#include <QString>
#include <QPlainTextEdit>

const QString GEMPTYFILE = "EMPTYFILE";

class LineNumberArea;

class LuaEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    LuaEdit(QWidget *parent, QString filename);
    QString filename() const { return m_filename; }
    void set_filename(QString filename) { m_filename = std::move(filename); }
    QAction *action() const { return m_window_menu_action; }
    void set_action(QAction *action) { m_window_menu_action = action; }

    void line_number_area_paint_event(QPaintEvent *event);
    int line_number_area_width();
    void reset_line_number_area_width() { update_line_number_area_width(0); }

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void update_line_number_area_width(int new_block_count);
    void highlight_current_line();
    void update_line_number_area(const QRect &rect, int dy);

private:
    bool load_from_file();
private:
    QSettings m_prefs;
    QString m_filename;
    Highlighter *m_highlighter;
    QAction *m_window_menu_action;
    QFont *m_font;
    QWidget *line_number_area;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(LuaEdit *editor) : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override { return QSize(m_editor->line_number_area_width(), 0); }

protected:
    void paintEvent(QPaintEvent *event) override { m_editor->line_number_area_paint_event(event); }

//signals:

//public slots:

private:
    LuaEdit *m_editor;
};


#endif // LUAEDIT_H
