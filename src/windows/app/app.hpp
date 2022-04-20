#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_app.h"
#include <QMainWindow>
#include <QObject>

#include "../../storage/storage.hpp"

class MainWindow : public QMainWindow, private Ui::MainWindow {
	Q_OBJECT

public:
	MainWindow(Store &store);

private:
	Store &store;

private slots:
	void btn_search_click();
	void btn_connect_click();
	void btn_create_share_click();

public slots:
	void device_connected();
	void share_linked();
	void my_shares_updated();
};

#endif // MAINWINDOW_H
