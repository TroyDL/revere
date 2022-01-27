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
#include "r_utils/r_nullable.h"
#include <memory>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct assignment_state
{
    std::string camera_id;
};
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

    // assignment
    void on_record_button_clicked();
    void on_rtsp_credentials_ok_clicked();
    void on_friendly_name_ok_clicked();
    void on_new_storage_clicked();
    void on_existing_storage_clicked();
    void on_retention_ok_clicked();

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

    r_utils::r_nullable<assignment_state> _assignmentState;

};
#endif
