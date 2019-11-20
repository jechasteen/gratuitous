# Todo

11-23-19 -> Got most of the AdvancedSearch functionality. Left off with trying to implement replace all functionality in AdvancedSearch

## Bugs

* Awesome commang shift+mod+c closes modified file without asking!

## File menu features

* New from template
	- Opens a dialog listing available "templates," e.g. copy the /etc/xdg/awesome/rc.lua and /usr/share/awesome/themes/default to a particular folder
	- Dialog also give the option of adding templates from a particular location (custom templates)
* Open folder? We will probably need to implement some sort of file tree for this to be feasible
* Revert file is a MUST

## Edit menu features

### Editor commands

* Find out if it is considered best practice to include cut/copy/paste/etc.
* Look at other popular editors/IDEs for feature-completeness


### Search

* Refactor quick search to use the new AdvancedSearch widget (static find function??)
* ~~Design the advanced search dock window widget~~
* Implement advanced search features
	- ~~Find~~ and replace
	- ~~Case sensitive and whole word~~
	- Search all open buffers
* ~~Add an advanced search button to the toolbar~~

### Preferences

* Design the prefs dialog
* Implement a config file in the user's home directory
* Replace all prefs TODOs in the code

## View menu features

### Preview

* Find out if Xephyr can reparent to a QMdiSubWindow, or maybe we can reparent to a top-level window and then 'demote' to a subwindow
* Implement Xephyr start/stop, awesome start/stop, and awesome SIGHUP (reload)
* Implement auto-reload on save and add a checkbox to the menu and toolbar

## Window menu features

* Create a separator between window actions and the list of open editors
* Bind next/previous QMdiSubWindow keysequences

## Help menu features

* Read through the lua docs taking notes, and condense a lua cheatsheet
* Also, examine the possibility of awesome-specific lua linting, refactoring/tidying, and static analysis
* It would be `awesome` to have qtcreator-style tooltips for awesome lib code, pulling from the docs
