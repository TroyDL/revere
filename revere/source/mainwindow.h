#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QListWidget>
#include <QTimer>
#include <QTabWidget>
#include "rtspcredentials.h"
#include "friendlyname.h"
#include "retention.h"
#include "neworexisting.h"
#include "r_disco/r_agent.h"
#include "r_disco/r_devices.h"
#include "r_vss/r_stream_keeper.h"
#include <memory>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setVisible(bool visible) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_camera_ui_update_timer();

private:
    Ui::MainWindow* _ui;

    QAction* _restoreAction;
    QAction* _quitAction;

    QSystemTrayIcon* _trayIcon;
    QMenu* _trayIconMenu;

    std::string _topDir;

    r_disco::r_agent _agent;
    r_disco::r_devices _devices;
    r_vss::r_stream_keeper _streamKeeper;

    QTimer* _cameraUIUpdateTimer;
    QListWidget* _discoveredListWidget;
    QListWidget* _recordingListWidget;
    RTSPCredentials* _rtspCredentials;
    FriendlyName* _friendlyName;
    Retention* _retention;
    NewOrExisting* _newOrExisting;
};
#endif
