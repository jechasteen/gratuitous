# Todo

11-23-19 -> Got most of the AdvancedSearch functionality. Left off with trying to implement replace all functionality in AdvancedSearch

## Bugs

* ~~Awesome command shift+mod+c closes modified file without asking!~~

## General Features

* Line numbering
* Tiled windows should optionally resize together
* Status bar with ~~status text~~ and current line/column number

## General Changes

* Quick search on separate toolbar

## *File* features

* New from template
	- Opens a dialog listing available "templates," e.g. copy the /etc/xdg/awesome/rc.lua and /usr/share/awesome/themes/default to a particular folder
	- Dialog also give the option of adding templates from a particular location (custom templates)
* Recent Files
* Open folder? We will probably need to implement some sort of file tree for this to be feasible
* ~~Revert file is a MUST~~
* ~~Open file with command line argument, e.g. `gratuitous ~/.config/awesome/rc.lua`~~

## *Edit* features

### Commands

* ~~Find out if it is considered best practice to include cut/copy/paste/etc.~~
* Look at other popular editors/IDEs for feature-completeness

### Features
	- Select All/None
	- Go to line
	- Display line number in status bar 

### Search

* ~~Refactor quick search to use the Search widget~~
* ~~Design the advanced search dock window widget~~
* Implement advanced search features
	- ~~Find~~ and ~~replace~~
	- ~~Case sensitive and whole word~~
	- Search all open buffers <- Later
* ~~Add an advanced search button to the toolbar~~

### Preferences

* ~~Design the prefs dialog~~
* ~~Implement a config file in the user's home directory~~
	- Use QSettings
	- Do we support system-wide AND user preferences?
* ~~Replace all prefs TODOs in the code~~

## *View* features

### Preview

* Find out if Xephyr can reparent to a QMdiSubWindow, or maybe we can reparent to a top-level window and then 'demote' to a subwindow
* ~~Implement Xephyr start/stop, awesome start/stop, and awesome SIGHUP (reload)~~
* ~~Implement auto-reload on save and add a checkbox to the menu and toolbar~~
* ~~Add preview window size to preferences and pass it when starting Xephyr~~

## *Window* features

* ~~Create a separator between window actions and the list of open editors~~
* ~~Bind next/previous QMdiSubWindow keysequences~~
* ~~Toggle tile horizontal/vertical~~ not supported :(
* ~~Toggle enabled/disabled window menu controls if there are editors open~~

## *Help* features

* Read through the lua docs taking notes, and condense a lua cheatsheet
* Also, examine the possibility of awesome-specific lua linting, refactoring/tidying, and static analysis
* It would be `awesome` to have qtcreator-style tooltips for awesome lib code, pulling from the docs
