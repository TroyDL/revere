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
    _newOrExisting(new NewOrExisting(this))
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




    auto button_box = _rtspCredentials->findChild<QDialogButtonBox*>("button_box");
    if(!button_box)
        R_THROW(("Unable to find button_box."));

    auto ok_button = button_box->button(QDialogButtonBox::Ok);
    if(!ok_button)
        R_THROW(("Unable to find ok_button."));

    auto cancel_button = button_box->button(QDialogButtonBox::Cancel);
    if(!cancel_button)
        R_THROW(("Unable to find cancel_button."));

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
    delete _rtspCredentials;

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
        b->setText("Assign");
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