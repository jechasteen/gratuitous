#ifndef LUAEDIT_H
#define LUAEDIT_H

#include "highlighter.h"

#include <utility>

#include <QAction>
#include <QDebug>
#include <QString>
#include <QTextEdit>

class LuaEdit : public QTextEdit
{
    Q_OBJECT
public:
    LuaEdit(QWidget *parent, QString filename);
    QString filename() const { return m_filename; }
    void set_filename(QString filename) { m_filename = std::move(filename); }
    QAction *action() const { return m_window_menu_action; }
    void set_action(QAction *action) { m_window_menu_action = action; }

private:
    void load_from_file();
private:
    QString m_filename;
    Highlighter *m_highlighter;
    QAction *m_window_menu_action;
    QFont *m_font;
};

#endif // LUAEDIT_H
