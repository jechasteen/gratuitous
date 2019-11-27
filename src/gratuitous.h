#ifndef GRATUITOUS_H
#define GRATUITOUS_H

#include "luaedit.h"
#include "prefs.h"
#include "preview.h"
#include "search.h"

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
#include <QSettings>
#include <QStatusBar>
#include <QString>
#include <QToolBar>
#include <QVector>
#include <QMdiSubWindow>

const QString VERSION = "0.1.0";

class Gratuitous : public QMainWindow
{
    Q_OBJECT

public:
    Gratuitous(QWidget *parent = nullptr, QString filename = "");
    ~Gratuitous() = default;

    void quit() { m_preview->stop(); }

private:
    // Utilities
    void do_save(LuaEdit* editor = nullptr);
    // void new_editor_window(QMdiSubWindow *window);
    void close_editor_window(QMdiSubWindow *window);
    LuaEdit *get_active_editor() { return m_mdi_area->activeSubWindow()->findChild<LuaEdit*>(); }
    static bool match_editor(QMdiSubWindow *window, QString filename) { return window->findChild<LuaEdit*>()->filename() == filename; }
    // TODO: overload this function, if needed, to support getting editors in other ways
    static LuaEdit *get_editor(QMdiSubWindow *window) { return window->findChild<LuaEdit*>(); }

    // data
    QSettings m_prefs;
    QMdiArea *m_mdi_area;

    // Widgets
    Search *m_search;
    Prefs *m_prefs_dialog;
    Preview *m_preview;
    QStatusBar *m_statusbar;

    QString m_default_path;
    int editor_count = 0;

private slots:
    void closeEvent(QCloseEvent *event) { exit(); event->accept(); }

    // File Action Slots
    void new_file() { ++editor_count; create_new_editor(GEMPTYFILE); }
    void open();
    void save() { do_save(); }
    void save_as();
    void save_all();
    void revert();
    void exit();

    // Edit Action Slots
    void undo() { get_active_editor()->undo(); }
    void redo() { get_active_editor()->redo(); }
    void cut() { get_active_editor()->cut(); }
    void copy() { get_active_editor()->copy(); }
    void paste() { get_active_editor()->paste(); }
    void toggle_search_dock_visible();
    void focus_quick_find() { m_toolbar->findChild<QLineEdit*>()->setFocus(); }
    void show_prefs_dialog();

    // View Action Slots
    void toggle_preview();
    void set_preview_menu_entries();
    void toggle_auto_reload();
    void reload_preview();

    // Window Action Slots
    void tile_windows() { m_mdi_area->tileSubWindows(); }
    void cascade_windows() { m_mdi_area->cascadeSubWindows(); }
    void next_window() { m_mdi_area->activateNextSubWindow(); }
    void prev_window() { m_mdi_area->activatePreviousSubWindow(); }
    void set_window_actions_enabled(bool state);
    void close_window();
    void close_all_windows();

    // Help Action Slots
    static void show_awesome_docs() { QDesktopServices::openUrl(QUrl("https://awesomewm.org/doc/api")); }
    void show_lua_reference() {}
    void show_about();

    // Other slots
    void create_new_editor(QString);
    void editor_was_modified(bool);
    void quick_search_next();
    void quick_search_previous();
    void handle_settings_change();
    void change_all_editors(QFont, bool);

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
    QAction *action_revert;
    QAction *action_exit;

    // Edit
    QAction *action_undo;
    QAction *action_redo;
    QAction *action_cut;
    QAction *action_copy;
    QAction *action_paste;
    QAction *action_show_find_dock;
    QAction *action_focus_quick_find;
    QAction *action_show_prefs_dialog;

    // View
    QAction *action_start_preview;
    QAction *action_stop_preview;
    QAction *action_toggle_auto_reload;
    QAction *action_reload_preview;

    // Window
    QAction *action_tile_windows;
    QAction *action_cascade_windows;
    QAction *action_next_window;
    QAction *action_prev_window;
    QAction *action_select_window;
    QAction *action_close_window;
    QAction *action_close_all_windows;

    // Help
    QAction *action_show_awesome_docs;
    QAction *action_show_lua_reference;
    QAction *action_show_about;
};
#endif // GRATUITOUS_H
