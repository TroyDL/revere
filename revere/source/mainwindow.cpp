#include "mainwindow.h"
#include "utils.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include "r_disco/r_stream_config.h"
#include "r_pipeline/r_gst_source.h"
#include "r_storage/r_storage_file.h"
#include "r_utils/r_time_utils.h"
#include <QMessageBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QtUiTools/quiloader.h>
#include <functional>

using namespace std;
using namespace r_utils;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _restoreAction(nullptr),
    _quitAction(nullptr),
    _trayIcon(nullptr),
    _trayIconMenu(nullptr),
    _topDir(top_dir()),
    _agent(_topDir),
    _devices(_topDir),
    _streamKeeper(_devices, _topDir),
    _cameraUIUpdateTimer(nullptr),
    _discoveredListWidget(nullptr),
    _recordingListWidget(nullptr),
    _rtspCredentials(new RTSPCredentials(this)),
    _friendlyName(new FriendlyName(this)),
    _retention(new Retention(this)),
    _newOrExisting(new NewOrExisting(this)),
    _assignmentState()
{
    r_pipeline::gstreamer_init();

    _ui->setupUi(this);

    _restoreAction = new QAction(tr("&Show"), this);
    connect(_restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    _quitAction = new QAction(tr("&Exit"), this);
    connect(_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    _trayIconMenu = new QMenu(this);
    _trayIconMenu->addAction(_restoreAction);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(_quitAction);

    _trayIcon = new QSystemTrayIcon(this);
    _trayIcon->setContextMenu(_trayIconMenu);
    QIcon icon = QIcon(":/images/R.png");
    _trayIcon->setIcon(icon);
    _trayIcon->show();

    _agent.set_stream_change_cb(bind(&r_disco::r_devices::insert_or_update_devices, &_devices, placeholders::_1));
    _agent.set_credential_cb(bind(&r_disco::r_devices::get_credentials, &_devices, placeholders::_1));
    _agent.set_is_recording_cb(bind(&r_vss::r_stream_keeper::is_recording, &_streamKeeper, placeholders::_1));

    _streamKeeper.start();
    _devices.start();
    _agent.start();

    _discoveredListWidget = findChild<QListWidget*>("discoveredListWidget");
    _discoveredListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    _recordingListWidget = findChild<QListWidget*>("recordingListWidget");

    _cameraUIUpdateTimer = new QTimer(this);
    connect(_cameraUIUpdateTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::on_camera_ui_update_timer));

    _cameraUIUpdateTimer->start(5000);




    auto button_box = _rtspCredentials->findChild<QDialogButtonBox*>("credentialsButtonBox");
    if(!button_box)
        R_THROW(("Unable to find button_box."));
    auto ok_button = button_box->button(QDialogButtonBox::Ok);
    if(!ok_button)
        R_THROW(("Unable to find ok_button."));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_rtsp_credentials_ok_clicked()));

    auto friendly_name_button_box = _friendlyName->findChild<QDialogButtonBox*>("friendlyNameButtonBox");
    if(!friendly_name_button_box)
        R_THROW(("Unable to find friendly_name_button_box."));
    ok_button = friendly_name_button_box->button(QDialogButtonBox::Ok);
    if(!ok_button)
        R_THROW(("Unable to find ok_button."));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_friendly_name_ok_clicked()));

    auto new_storage_button = _newOrExisting->findChild<QPushButton*>("newStorageButton");
    if(!new_storage_button)
        R_THROW(("Unable to find new_storage_button."));
    connect(new_storage_button, SIGNAL(clicked()), this, SLOT(on_new_storage_clicked()));

    auto existing_storage_button = _newOrExisting->findChild<QPushButton*>("existingStorageButton");
    if(!existing_storage_button)
        R_THROW(("Unable to find existing_storage_button."));
    connect(existing_storage_button, SIGNAL(clicked()), this, SLOT(on_existing_storage_clicked()));

    auto retention_button_box = _retention->findChild<QDialogButtonBox*>("retentionButtonBox");
    if(!retention_button_box)
        R_THROW(("Unable to find retention_button_box."));
    ok_button = retention_button_box->button(QDialogButtonBox::Ok);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_retention_ok_clicked()));

    // hookup _retention slider
    // _retention->findChild<QSlider*>("retentionSlider")
    // _retention->findChild<QLineEdit*>("maxDaysRetention")
    // _retention->findChild<QLabel*>("numDaysLabel")
    // _retention->findChild<QLabel*>("fileSizeLabel")


    // Assignment Process
    //   1. Popup _rtspCredentials dialog
    //   2. If OK, then use fetch_sdp_media() to fetch media info.
    //   3. Use fetch_bytes_per_second() to fetch bitrate.
    //   4. use r_devices::get_camera_by_id() to fetch camera.
    //   5. Update camera with media info and bitrate and use r_devices::save_camera()
    //   6. Popup _friendlyName dialog
    //   7. Update, r_camera friendly_name
    //   8. Save camera
    //   9. Popup _newOrExisting dialog
    //   10. If existing, popup file find dialog and collect filename.
    //   11. Else if new, popup _retention dialog and collect retention info, popup new file dialog and collect filename.
    //       create r_storage_file with filename and retention info
    //   12. Update r_camera with r_storage_file path etc...
    //   13. Update r_camera state to assigned
    //   14. Save camera

    //_rtspCredentials->show();
    //_friendlyName->show();
    //_retention->show();
    //_newOrExisting->show();
}

MainWindow::~MainWindow()
{
    _rtspCredentials->hide();
    delete _rtspCredentials;

    _friendlyName->hide();
    delete _friendlyName;

    _newOrExisting->hide();
    delete _newOrExisting;

    _retention->hide();
    delete _retention;

    _cameraUIUpdateTimer->stop();
    delete _cameraUIUpdateTimer;

    _agent.stop();
    _devices.stop();
    _streamKeeper.stop();

    delete _ui;
}

void MainWindow::setVisible(bool visible)
{
    _restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_MACOS
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (_trayIcon->isVisible()) {
        QMessageBox::information(this, tr("Minmize to tray"),
                                 tr("The program will keep running in the system tray. To terminate the program, "
                                    "choose <b>Exit</b> in the Revere context menu."));
        hide();
        event->ignore();
    }
}

void MainWindow::on_camera_ui_update_timer()
{
    printf("on_camera_ui_update_timer()\n");

    auto assigned_cameras = _devices.get_assigned_cameras();

    map<string, r_disco::r_camera> assigned;
    for(auto& c: assigned_cameras)
        assigned[c.id] = c;

    _recordingListWidget->clear();

    for(auto& c: assigned)
        _recordingListWidget->addItem(QString::fromStdString(c.second.camera_name.value()));

    auto all_cameras = _devices.get_all_cameras();

    _discoveredListWidget->clear();

    for(int i = 0; i < all_cameras.size(); ++i)
    {
        auto c = all_cameras[i];

        if(assigned.find(c.id) != assigned.end())
            continue;

        QWidget* w = new QWidget(_discoveredListWidget);
        QLabel* l = new QLabel(QString::fromStdString(c.camera_name.value()), w);
        QPushButton* b = new QPushButton(w);
        b->setText("Record");
        b->setProperty("camera_id", QString::fromStdString(c.id));
        connect(b, SIGNAL(clicked()), this, SLOT(on_record_button_clicked()));
        QHBoxLayout* layout = new QHBoxLayout(w);
        layout->addWidget(b);
        layout->addWidget(l);
        layout->setSizeConstraint(QLayout::SetFixedSize);
        w->setLayout(layout);
        QListWidgetItem* item = new QListWidgetItem(_discoveredListWidget);
        item->setSizeHint(w->sizeHint());
        _discoveredListWidget->insertItem(i, item);
        _discoveredListWidget->setItemWidget(item, w);
    }
}

void MainWindow::on_record_button_clicked()
{
    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());

    if(!sender)
        R_THROW(("Unable to cast sender to QPushButton."));

    auto camera_id = sender->property("camera_id").toString().toStdString();

    assignment_state as;
    as.camera_id = camera_id;
    _assignmentState.set_value(as);

    _rtspCredentials->show();
}

void MainWindow::on_rtsp_credentials_ok_clicked()
{
    printf("on_rtsp_credentials_clicked()\n");

    _rtspCredentials->hide();

    // _rtspCredentials->findChild<QLabel*>("ipAddressLabel")
    // _rtspCredentials->findChild<QLineEdit*>("usernameLineEdit")
    // _rtspCredentials->findChild<QLineEdit*>("passwordLineEdit")

    // add new method to r_pipeline that does 3 things (and takes credentials)
    //     1) fetch the sdp media info
    //     2) play the stream briefly and fetch the bitrate
    //     3) grab the first key frame
    //     4) return the media info, bitrate and key frame
    // populate _assignmentState with credentials, camera_name, codec, bitrate and key frame
    // populate _friendlyName with camera info: camera_name, codec, bitrate and key frame

    _friendlyName->show();
}

void MainWindow::on_friendly_name_ok_clicked()
{
    printf("on_friendly_name_ok_clicked()\n");
    _friendlyName->hide();
    // _friendlyName->findChild<QLabel*>("cameraNameLabel")
    // _friendlyName->findChild<QWidget*>("imageContainer")
    // _friendlyName->findChild<QLineEdit*>("friendlyNameLineEdit")
    // populate _assignmentState with friendly_name
    _newOrExisting->show();
}

void MainWindow::on_new_storage_clicked()
{
    printf("on_new_storage_clicked()\n");
    _newOrExisting->hide();


    _retention->show();
}

void MainWindow::on_existing_storage_clicked()
{
    printf("on_existing_storage_clicked()\n");
    // pop load file dialog
}

void MainWindow::on_retention_ok_clicked()
{
    // _retention->findChild<QSlider*>("retentionSlider")
    // _retention->findChild<QLineEdit*>("maxDaysRetention")
    // _retention->findChild<QLabel*>("numDaysLabel")
    // _retention->findChild<QLabel*>("fileSizeLabel")

    printf("on_retention_ok_clicked()\n");
}
