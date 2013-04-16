#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QLabel;
class QMenu;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

private:
	void createActions();
	void createMenus();

	QMenu *fileMenu;
	QMenu *helpMenu;
	QAction *exitAct;
	QAction *aboutQtAct;
};

#endif
