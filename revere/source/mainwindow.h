#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "r_disco/r_agent.h"
#include "r_disco/r_devices.h"
#include "r_vss/r_stream_keeper.h"
#include "r_utils/r_nullable.h"
#include "r_pipeline/r_stream_info.h"
#include "rtspcredentials.h"
#include "friendlyname.h"
#include "retention.h"
#include "neworexisting.h"
#include "pleasewait.h"
#include "newfilename.h"
#include "motiondetection.h"
#include "ws.h"
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QListWidget>
#include <QTimer>
#include <QTabWidget>
#include <memory>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct assignment_state
{
    std::string camera_id;
    std::string ipv4;
    r_utils::r_nullable<std::string> rtsp_username;
    r_utils::r_nullable<std::string> rtsp_password;
    std::string camera_friendly_name;
    int64_t byte_rate {64000};
    int continuous_retention_days {3};
    int motion_retention_days {10};
    int motion_percentage_estimate {5};
    std::string file_name;
    r_utils::r_nullable<int64_t> num_storage_file_blocks {0};
    r_utils::r_nullable<int64_t> storage_file_block_size {0};
    r_utils::r_nullable<r_disco::r_camera> camera;
    std::map<std::string, r_pipeline::r_sdp_media> sdp_medias;
    std::vector<uint8_t> key_frame;
    bool do_motion_detection;
    std::string motion_detection_file_path;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setVisible(bool visible) override;

    void save_window_state();
    void restore_window_state();

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void fetch_camera_params_done();

private slots:
    void on_camera_ui_update_timer();

    void on_record_list_item_clicked(QListWidgetItem *item);
    void on_record_button_clicked();
    void on_remove_button_clicked();
    void on_rtsp_credentials_ok_clicked();
    void on_fetch_camera_params_done();
    void on_friendly_name_ok_clicked();
    void on_motion_detection_ok_clicked();
    void on_new_storage_clicked();
    void on_existing_storage_clicked();
    void on_retention_ok_clicked();
    void on_continuous_retention_days_changed(QString value);
    void on_motion_retention_days_changed(QString value);
    void on_percentage_estimate_changed(QString value);
    void on_new_filename_ok_clicked();

private:
    void _update_list_ui();
    void _update_retention_ui();

    void _assign_camera(const assignment_state& as);

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
    PleaseWait* _pleaseWait;
    NewFileName* _newFileName;
    MotionDetection* _motionDetection;

    r_utils::r_nullable<assignment_state> _assignmentState;

    ws _ws;
};
#endif
