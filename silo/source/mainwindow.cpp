#include "mainwindow.h"
#include "utils.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include "r_disco/r_stream_config.h"
#include "r_pipeline/r_gst_source.h"
#include "r_storage/r_storage_file.h"
#include "r_utils/r_time_utils.h"
#include <QMessageBox>
#include <QCloseEvent>
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
    _discoveredTable(nullptr),
    _discoveredTimer(nullptr)
{
    r_pipeline::gstreamer_init();

    _ui->setupUi(this);

    _restoreAction = new QAction(tr("&Show"), this);
    connect(_restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    _quitAction = new QAction(tr("&Quit"), this);
    connect(_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    _trayIconMenu = new QMenu(this);
    _trayIconMenu->addAction(_restoreAction);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(_quitAction);

    _trayIcon = new QSystemTrayIcon(this);
    _trayIcon->setContextMenu(_trayIconMenu);
    QIcon icon = QIcon(":/images/heart.png");
    _trayIcon->setIcon(icon);
    setWindowIcon(icon);
    _trayIcon->show();

    _agent.set_stream_change_cb(bind(&r_disco::r_devices::insert_or_update_devices, &_devices, placeholders::_1));
    _agent.set_credential_cb(bind(&r_disco::r_devices::get_credentials, &_devices, placeholders::_1));
    _agent.set_is_recording_cb(bind(&r_vss::r_stream_keeper::is_recording, &_streamKeeper, placeholders::_1));

    // temporary link to function that just prints out stream stats.
    QPushButton* btn = findChild<QPushButton*>("pushButton");
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(button_pushed()));

    _discoveredTable = findChild<QTableWidget*>("discoveredTable");
    _discoveredTable->setColumnCount(1);

    _discoveredTimer = new QTimer(this);
    connect(_discoveredTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::on_discovered_timer_timeout));

    _mainTabWidget = findChild<QTabWidget*>("mainTabWidget");
    connect(_mainTabWidget, &QTabWidget::currentChanged, this, &MainWindow::on_tab_changed);

    _mainTabWidget->setCurrentIndex(0);

    _streamKeeper.start();
    _devices.start();
    _agent.start();
}

MainWindow::~MainWindow()
{
    _discoveredTimer->stop();
    delete _discoveredTimer;

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
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}

vector<pair<int64_t, int64_t>> find_contiguous_segments(const vector<int64_t>& times)
{
    // Since our input times are key frames and different cameras may have different GOP sizes we need to discover for
    // each camera what the normal space between key frames is. We then multiply this value by 1.5 to find our segment
    // threshold.

    int64_t threshold = 0;
    if(times.size() > 1)
    {
        vector<int64_t> deltas;
        adjacent_difference(begin(times), end(times), back_inserter(deltas));
        threshold = (deltas.size() > 2)?(int64_t)((accumulate(begin(deltas)+1, end(deltas), .0) / deltas.size()) * 1.5):0;
    }

    printf("threshold: %ld\n", threshold);

    deque<int64_t> timesd;
    for (auto t : times) {
        timesd.push_back(t);
    }

    vector<vector<int64_t>> segments;
    vector<int64_t> current;

    // Walk the times front to back appending new times to current as long as they are within the threshold.
    // If a new time is outside the threshold then we have a new segment so push current onto segments and
    // start a new current.

    while(!timesd.empty())
    {
        auto t = timesd.front();
        timesd.pop_front();

        if(current.empty())
            current.push_back(t);
        else
        {
            auto delta = t - current.back();
            if(delta > threshold && !current.empty())
            {
                segments.push_back(current);
                current = {t};
            }
            else current.push_back(t);
        }
    }

    // since we push current when we find a gap bigger than threshold we need to push the last current segment.
    if(!current.empty())
        segments.push_back(current);

    vector<pair<int64_t, int64_t>> output;
    for(auto& s : segments)
        output.push_back(make_pair(s.front(), s.back()));

    return output;
}

void MainWindow::button_pushed()
{
    try
    {
        auto stream_status = _streamKeeper.fetch_stream_status();

        for(auto& s : stream_status)
        {
            r_storage::r_storage_file sf("/home/td/Documents/revere/silo/video/" + s.camera.record_file_path.value());
            auto kfst = sf.key_frame_start_times(r_storage::R_STORAGE_MEDIA_TYPE_VIDEO);

//            for(auto b = kfst.begin()+1, e = kfst.end(); b != e; ++b)
//            {
//                auto ts = *b;
//                auto ts_prev = *(b-1);

//                auto ts_diff = ts - ts_prev;

//                printf("%ld, ", ts_diff);
//            }
//            printf("\n");

            auto segments = find_contiguous_segments(kfst);

            string seg_str;
            for(auto s : segments)
                seg_str += "[" + r_string_utils::int64_to_s((s.second-s.first)/1000) + "], ";
                //seg_str += "[" + to_string(s.first) + ", " + to_string(s.second) + "], ";
                

            printf("camera: id=%s, bytes_per_second=%u, %s\n", s.camera.id.c_str(), s.bytes_per_second, seg_str.c_str());
        }
    }
    catch(exception& ex)
    {
        R_LOG_ERROR("exception: %s", ex.what());
        printf("exception: %s\n", ex.what());
    }
}

void MainWindow::on_discovered_timer_timeout()
{
    printf("on discovered timer timeout\n");
    auto cameras = _devices.get_all_cameras();
    printf("cameras: %lu\n", cameras.size());

    while(_discoveredTable->rowCount() > 0)
        _discoveredTable->removeRow(0);

    for(int i = 0; i < cameras.size(); ++i)
    {
        auto c = cameras[i];

        printf("c.ipv4: %s\n",c.ipv4.value().c_str());
        _discoveredTable->insertRow(_discoveredTable->rowCount());
#if 0
        QWidget* w = new QWidget();
        QLabel* l = new QLabel(QString::fromStdString(c.ipv4.value()));
        QHBoxLayout* layout = new QHBoxLayout(w);
        layout->addWidget(l);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        w->setLayout(layout);
        _discoveredTable->setCellWidget(i, 0, w);
#endif
        _discoveredTable->setItem(
            _discoveredTable->rowCount()-1, 
            0,
            new QTableWidgetItem(QString::fromStdString(c.ipv4.value()))
        );
    }
}

void MainWindow::on_tab_changed(int index)
{
    printf("on tab changed: %d\n", index);
    if(index == 1)
    {
        _discoveredTimer->start(5000);
    }
    else if(index == 0)
    {
        _discoveredTimer->stop();
    }
}
