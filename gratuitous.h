#ifndef GRATUITOUS_H
#define GRATUITOUS_H

#include "luaedit.h"
#include "prefs.h"

#include <utility>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDockWidget>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QToolBar>
#include <QVector>
#include <QMdiSubWindow>

const QString VERSION = "0.01.00";

class Gratuitous : public QMainWindow
{
    Q_OBJECT

public:
    Gratuitous(QWidget *parent = nullptr);
    ~Gratuitous() = default;

private:
    // Utilities
    void get_default_path();
    void do_save(LuaEdit *editor = nullptr);
    void new_editor_window(QMdiSubWindow *window);
    void close_editor_window(QMdiSubWindow *window);
    LuaEdit *get_active_editor() { return m_mdi_area->activeSubWindow()->findChild<LuaEdit*>(); }
    static bool match_editor(QMdiSubWindow *window, QString filename) { return window->findChild<LuaEdit*>()->filename() == filename; }
    // TODO: overload this function, if needed, to support getting editors in other ways
    static LuaEdit *get_editor(QMdiSubWindow *window) { return window->findChild<LuaEdit*>(); }

    // data
    QMdiArea *m_mdi_area;
    QDockWidget *m_search_dock;

    QString m_default_path;
    Prefs *m_prefs;
    int editor_count = 0;

private slots:
    // File Action Slots
    void new_file() { ++editor_count; create_new_editor(""); }
    void open();
    void save() { do_save(); }
    void save_as();
    void save_all();
    void exit();

    // Edit Action Slots
    void toggle_search_dock_visible();
    void focus_quick_find() { m_toolbar->findChild<QLineEdit*>()->setFocus(); }
    void show_prefs_dialog();

    // View Action Slots
    void show_preview() {}
    void reload_preview() {}

    // Window Action Slots
    void tile_windows() { m_mdi_area->tileSubWindows(); }
    void cascade_windows() { m_mdi_area->cascadeSubWindows(); }
    void close_window();
    void close_all_windows() { m_mdi_area->closeAllSubWindows(); }

    // Help Action Slots
    static void show_awesome_docs() { QDesktopServices::openUrl(QUrl("https://awesomewm.org/doc/api")); }
    void show_lua_reference() {}
    void show_about();

    // Other slots
    void create_new_editor(QString filename);
    void editor_was_modified(bool changed);
    void handle_quick_search();
    void quick_search_next();
    void quick_search_previous();

signals:
    void triggered(QAction *);

private:
    /*
     * Menu/Toolbar/Action stuff
     */
    void setup_menu_actions();
    void add_menus();
    void setup_toolbar();

    QMenu *menu_file;
    QMenu *menu_edit;
    QMenu *menu_view;
    QMenu *menu_window;
    QMenu *menu_help;

    QToolBar *m_toolbar;

    /*
     * Actions
     */

    // File
    QAction *action_new;
    QAction *action_open;
    QAction *action_save;
    QAction *action_save_as;
    QAction *action_save_all;
    QAction *action_exit;

    // Edit
    QAction *action_show_find_dock;
    QAction *action_focus_quick_find;
    QAction *action_show_prefs_dialog;
    // TODO: do we want to have cut, copy, paste, etc. in this menu?

    // View
    QAction *action_show_preview;
    QAction *action_reload_preview;

    // Window
    QAction *action_tile_windows;
    QAction *action_cascade_windows;
    QAction *action_select_window;
    QAction *action_close_window;
    QAction *action_close_all_windows;

    // Help
    QAction *action_show_awesome_docs;
    QAction *action_show_lua_reference;
    QAction *action_show_about;
};
#endif // GRATUITOUS_H
