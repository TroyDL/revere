#include "mainwindow.h"
#include "utils.h"
#include "ui_mainwindow.h"
#include "r_disco/r_stream_config.h"
#include "r_pipeline/r_gst_source.h"
#include "r_storage/r_storage_file.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_file.h"
#include "r_codec/r_video_decoder.h"
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLayout>
#include <functional>
#include <thread>

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
    _pleaseWait(new PleaseWait(this)),
    _newFileName(new NewFileName(this)),
    _assignmentState()
{
    r_pipeline::gstreamer_init();

    _ui->setupUi(this);

    connect(this, &MainWindow::fetch_camera_params_done, this, &MainWindow::on_fetch_camera_params_done);

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
    auto ok_button = button_box->button(QDialogButtonBox::Ok);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_rtsp_credentials_ok_clicked()));
    auto cancel_button = button_box->button(QDialogButtonBox::Cancel);
    connect(cancel_button, SIGNAL(clicked()), _rtspCredentials, SLOT(close()));

    auto friendly_name_button_box = _friendlyName->findChild<QDialogButtonBox*>("friendlyNameButtonBox");
    ok_button = friendly_name_button_box->button(QDialogButtonBox::Ok);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_friendly_name_ok_clicked()));
    cancel_button = friendly_name_button_box->button(QDialogButtonBox::Cancel);
    connect(cancel_button, SIGNAL(clicked()), _friendlyName, SLOT(close()));

    auto new_storage_button = _newOrExisting->findChild<QPushButton*>("newStorageButton");
    connect(new_storage_button, SIGNAL(clicked()), this, SLOT(on_new_storage_clicked()));

    auto existing_storage_button = _newOrExisting->findChild<QPushButton*>("existingStorageButton");
    connect(existing_storage_button, SIGNAL(clicked()), this, SLOT(on_existing_storage_clicked()));

    auto retention_button_box = _retention->findChild<QDialogButtonBox*>("retentionButtonBox");
    ok_button = retention_button_box->button(QDialogButtonBox::Ok);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_retention_ok_clicked()));
    cancel_button = retention_button_box->button(QDialogButtonBox::Cancel);
    connect(cancel_button, SIGNAL(clicked()), _retention, SLOT(close()));

    auto daysContinuousRetention = _retention->findChild<QLineEdit*>("daysContinuousRetention");
    connect(daysContinuousRetention, SIGNAL(textChanged(QString)), this, SLOT(on_continuous_retention_days_changed(QString)));

    auto daysMotionRetention = _retention->findChild<QLineEdit*>("daysMotionRetention");
    connect(daysMotionRetention, SIGNAL(textChanged(QString)), this, SLOT(on_motion_retention_days_changed(QString)));

    auto motionPercentageEstimate = _retention->findChild<QLineEdit*>("motionPercentageEstimate");
    connect(motionPercentageEstimate, SIGNAL(textChanged(QString)), this, SLOT(on_percentage_estimate_changed(QString)));
}

MainWindow::~MainWindow()
{
    _pleaseWait->hide();
    delete _pleaseWait;

    _newFileName->hide();
    delete _newFileName;

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

    _trayIcon->setVisible(false);
    delete _trayIcon;

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
    if(_trayIcon->isVisible())
    {
        QMessageBox::information(this, tr("Minmize to tray"),
                                 tr("The program will keep running in the system tray. To terminate the program, "
                                    "choose <b>Exit</b> in the Revere context menu."));
        hide();
        event->ignore();
    }
}

void MainWindow::on_camera_ui_update_timer()
{
    auto assigned_cameras = _devices.get_assigned_cameras();

    map<string, r_disco::r_camera> assigned;
    for(auto& c: assigned_cameras)
        assigned[c.id] = c;

    _recordingListWidget->clear();

    int i = 0;
    for(auto& c: assigned)
    {
        //_recordingListWidget->addItem(QString::fromStdString(c.second.camera_name.value()));
        QWidget* w = new QWidget(_recordingListWidget);
        QHBoxLayout* hl = new QHBoxLayout(w);
        hl->setSizeConstraint(QLayout::SetFixedSize);
        w->setLayout(hl);
        QPushButton* b = new QPushButton(w);
        b->setText("Remove");
        b->setProperty("camera_id", QString::fromStdString(c.first));
        connect(b, SIGNAL(clicked()), this, SLOT(on_remove_button_clicked()));

        QWidget* lc = new QWidget(_recordingListWidget);
        QVBoxLayout* vl = new QVBoxLayout(lc);
        lc->setLayout(vl);
        QLabel* camera_name_l = new QLabel(QString::fromStdString(c.second.friendly_name.value()), lc);
        QLabel* camera_ip_l = new QLabel(QString::fromStdString(c.second.ipv4.value()), lc);
        vl->addWidget(camera_name_l);
        vl->addWidget(camera_ip_l);

        hl->addWidget(b);
        hl->addWidget(lc);

        QListWidgetItem* item = new QListWidgetItem(_recordingListWidget);
        item->setSizeHint(w->sizeHint());
        _recordingListWidget->insertItem(i, item);
        _recordingListWidget->setItemWidget(item, w);
        ++i;
    }

    auto all_cameras = _devices.get_all_cameras();

    _discoveredListWidget->clear();

    for(int i = 0; i < all_cameras.size(); ++i)
    {
        auto c = all_cameras[i];

        if(assigned.find(c.id) != assigned.end())
            continue;

        QWidget* w = new QWidget(_discoveredListWidget);
        QHBoxLayout* hl = new QHBoxLayout(w);
        hl->setSizeConstraint(QLayout::SetFixedSize);
        w->setLayout(hl);
        QPushButton* b = new QPushButton(w);
        b->setText("Record");
        b->setProperty("camera_id", QString::fromStdString(c.id));
        connect(b, SIGNAL(clicked()), this, SLOT(on_record_button_clicked()));

        QWidget* lc = new QWidget(_discoveredListWidget);
        QVBoxLayout* vl = new QVBoxLayout(lc);
        lc->setLayout(vl);
        QLabel* camera_name_l = new QLabel(QString::fromStdString(c.camera_name.value()), lc);
        QLabel* camera_ip_l = new QLabel(QString::fromStdString(c.ipv4.value()), lc);
        vl->addWidget(camera_name_l);
        vl->addWidget(camera_ip_l);

        hl->addWidget(b);
        hl->addWidget(lc);

        QListWidgetItem* item = new QListWidgetItem(_discoveredListWidget);
        item->setSizeHint(w->sizeHint());
        _discoveredListWidget->insertItem(i, item);
        _discoveredListWidget->setItemWidget(item, w);
    }
}

void MainWindow::on_record_button_clicked()
{
    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());

    auto camera_id = sender->property("camera_id").toString().toStdString();

    assignment_state as;
    as.camera_id = camera_id;

    auto maybe_camera = _devices.get_camera_by_id(camera_id);

    as.camera = maybe_camera;
    as.ipv4 = maybe_camera.value().ipv4.value();

    _assignmentState.set_value(as);

    _rtspCredentials->findChild<QLabel*>("ipAddressLabel")->setText(QString::fromStdString(as.ipv4));

    _rtspCredentials->show();
}

void MainWindow::on_remove_button_clicked()
{
    QPushButton* sender = qobject_cast<QPushButton*>(QObject::sender());

    auto camera_id = sender->property("camera_id").toString().toStdString();

    auto maybe_c = _devices.get_camera_by_id(camera_id);

    if(!maybe_c.is_null())
        _devices.remove_camera(maybe_c.value());
}

void MainWindow::on_rtsp_credentials_ok_clicked()
{
    auto as = _assignmentState.value();
    auto username = _rtspCredentials->findChild<QLineEdit*>("usernameLineEdit")->text().toStdString();
    auto password = _rtspCredentials->findChild<QLineEdit*>("passwordLineEdit")->text().toStdString();
    as.rtsp_username = (username.empty())?r_nullable<string>():r_nullable<string>(username);
    as.rtsp_password = (password.empty())?r_nullable<string>():r_nullable<string>(password);

    _rtspCredentials->hide();

    auto camera = as.camera.value();

    _agent.interrogate_camera(
        camera.camera_name.value(),
        camera.ipv4.value(),
        camera.xaddrs.value(),
        camera.address.value(),
        as.rtsp_username,
        as.rtsp_password
    );

    as.camera = _devices.get_camera_by_id(as.camera_id);

    _assignmentState.set_value(as);

    std::thread th([this](){
        auto as = this->_assignmentState.value();

        auto cp = r_pipeline::fetch_camera_params(as.camera.value().rtsp_url.value(), as.rtsp_username, as.rtsp_password);

        as.byte_rate = cp.bytes_per_second;
        as.sdp_medias = cp.sdp_medias;
        as.key_frame = cp.video_key_frame;

        this->_assignmentState.set_value(as);

        this->_pleaseWait->hide();

        emit this->fetch_camera_params_done();
    });
    th.detach();

    _pleaseWait->findChild<QLabel*>("messageLabel")->setText(QString("Please wait while we communicate with your camera..."));
    _pleaseWait->show();
}

void MainWindow::on_fetch_camera_params_done()
{
    _friendlyName->findChild<QLabel*>("cameraNameLabel")->setText(QString::fromStdString(_assignmentState.value().camera.value().camera_name.value()));
    _friendlyName->show();
}

void MainWindow::on_friendly_name_ok_clicked()
{
    auto as = _assignmentState.value();
    as.camera_friendly_name = _friendlyName->findChild<QLineEdit*>("friendlyNameLineEdit")->text().toStdString();
    _assignmentState.set_value(as);

    _friendlyName->hide();
    // _friendlyName->findChild<QLabel*>("cameraNameLabel")
    // _friendlyName->findChild<QWidget*>("imageContainer")
    // _friendlyName->findChild<QLineEdit*>("friendlyNameLineEdit")
    // populate _assignmentState with friendly_name
    _newOrExisting->show();
}

void MainWindow::on_new_storage_clicked()
{
    _newOrExisting->hide();


    _retention->show();
    update_retention_ui();
}

void MainWindow::on_existing_storage_clicked()
{
    printf("on_existing_storage_clicked()\n");
    _newOrExisting->hide();

    auto fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        QString::fromStdString(sub_dir("video")),
        tr("Revere Video Database (*.rvd)")
    ).toStdString();


    fileName = fileName.substr(fileName.rfind(r_fs::PATH_SLASH)+1);

    auto as = _assignmentState.value();

    auto c = as.camera.value();

    c.rtsp_username = as.rtsp_username;
    c.rtsp_password = as.rtsp_password;
    c.friendly_name = as.camera_friendly_name;
    c.record_file_path = fileName;
    //c.n_record_file_blocks = as.num_storage_file_blocks;
    //c.record_file_block_size = as.storage_file_block_size;
    c.state = "assigned";

    _devices.save_camera(c);

//    QFileDialog dialog(this, "Open File");
//    dialog.setFileMode(QFileDialog::ExistingFile);
//    dialog.setWindowTitle(QObject::tr("Open Existing..."));
//    dialog.setDirectory(QString::fromStdString(sub_dir("video")));
//    dialog.exec();


}

static string _make_file_name(string name)
{
    replace(begin(name), end(name), ' ', '_');
    return name + ".rvd";
}

void MainWindow::on_retention_ok_clicked()
{
    _retention->hide();

    auto as = _assignmentState.value();

    auto fileNameEdit = _newFileName->findChild<QLineEdit*>("fileNameLineEdit");
    fileNameEdit->setText(QString::fromStdString(_make_file_name(as.camera_friendly_name)));

    auto button_box = _newFileName->findChild<QDialogButtonBox*>("buttonBox");
    auto ok_button = button_box->button(QDialogButtonBox::Ok);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(on_new_filename_ok_clicked()));
    auto cancel_button = button_box->button(QDialogButtonBox::Cancel);
    connect(cancel_button, SIGNAL(clicked()), _newFileName, SLOT(close()));

    _newFileName->show();
}

void MainWindow::on_continuous_retention_days_changed(QString value)
{
    auto as = _assignmentState.value();
    as.continuous_retention_days = value.toInt();
    _assignmentState.set_value(as);

    update_retention_ui();
}

void MainWindow::on_motion_retention_days_changed(QString value)
{
    auto as = _assignmentState.value();
    as.motion_retention_days = value.toInt();
    _assignmentState.set_value(as);

    update_retention_ui();
}

void MainWindow::on_percentage_estimate_changed(QString value)
{
    auto as = _assignmentState.value();
    as.motion_percentage_estimate = value.toInt();
    _assignmentState.set_value(as);

    update_retention_ui();
}

void MainWindow::update_retention_ui()
{
    auto as = _assignmentState.value();

    _retention->findChild<QLabel*>("headingLabel")->setText(QString::fromStdString(as.camera_friendly_name + " at " + to_string((as.byte_rate * 8) / 1024)) + " kbps.\n");

    auto continuous_sz_info = r_storage::r_storage_file::required_file_size_for_retention_hours((as.continuous_retention_days*24), as.byte_rate);

    auto motion_sz_info = r_storage::r_storage_file::required_file_size_for_retention_hours((as.motion_retention_days*24), as.byte_rate);

    double motionPercentage = ((double)as.motion_percentage_estimate) / 100.0;

    int64_t n_blocks = continuous_sz_info.first + (motionPercentage*(double)motion_sz_info.first);

    as.num_storage_file_blocks = n_blocks;
    as.storage_file_block_size = continuous_sz_info.second;

    auto human_readable_size = r_storage::r_storage_file::human_readable_file_size(as.num_storage_file_blocks * as.storage_file_block_size);

    auto fileSizeLabel = _retention->findChild<QLabel*>("fileSizeLabel");

    fileSizeLabel->setText(QString::fromStdString(human_readable_size));

    _assignmentState.set_value(as);
}

void MainWindow::on_new_filename_ok_clicked()
{
    _newFileName->hide();

    auto video_path = sub_dir("video");

    auto as = _assignmentState.value();

    auto c = as.camera.value();

    auto fileName = _newFileName->findChild<QLineEdit*>("fileNameLineEdit")->text().toStdString();

    auto storage_path = join_path(video_path, fileName);

    printf("on_new_filename_ok_clicked(): %s\n", storage_path.c_str());

    r_storage::r_storage_file::allocate(storage_path, as.storage_file_block_size, as.num_storage_file_blocks);

    c.rtsp_username = as.rtsp_username;
    c.rtsp_password = as.rtsp_password;
    c.friendly_name = as.camera_friendly_name;
    c.record_file_path = fileName;
    c.n_record_file_blocks = as.num_storage_file_blocks;
    c.record_file_block_size = as.storage_file_block_size;
    c.state = "assigned";

    _devices.save_camera(c);

    // -make the record file with the as settings
    //   r_storage_file::allocate(file_name, block_size, n_blocks);
    // -set rtsp_username, rtsp_password, friendly_name, record_file_path, n_record_file_blocks, record_file_block_size
    //     on as.camera
    // -set state=assigned on as.camera
    // -save camera


    //std::string ipv4;
    //r_utils::r_nullable<std::string> rtsp_username;
    //r_utils::r_nullable<std::string> rtsp_password;
    //std::string camera_friendly_name;
    //int64_t byte_rate {64000};
    //int continuous_retention_days {3};
    //int motion_retention_days {10};
    //int motion_percentage_estimate {5};
    //int64_t num_storage_file_blocks {0};
    //int64_t storage_file_block_size {0};
    //std::string storage_path;
    //r_utils::r_nullable<r_disco::r_camera> camera;
    //std::map<std::string, r_pipeline::r_sdp_media> sdp_medias;
    //std::vector<uint8_t> key_frame;

    _assignmentState.set_value(as);


}
