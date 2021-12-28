#include "mainwindow.h"
#include "utils.h"
#include "ui_mainwindow.h"
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
    _streamKeeper(_devices, _topDir)
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

    _streamKeeper.start();
    _devices.start();
    _agent.start();
}

MainWindow::~MainWindow()
{
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
    vector<pair<int64_t, int64_t>> segments;

    int64_t threshold = 0;
    if(times.size() > 1)
    {
        vector<int64_t> deltas;
        adjacent_difference(begin(times), end(times), back_inserter(deltas));
        threshold = (deltas.size() > 2)?(int64_t)((accumulate(begin(deltas)+1, end(deltas), .0) / deltas.size()) * 1.5):0;
    }

    deque<int64_t> timesd;
    for (auto t : times) {
        timesd.push_back(t);
    }

    printf("threshold: %ld\n", threshold);

#if 1
    size_t n_times = times.size();

    if(n_times == 1)
        segments.push_back(make_pair(times[0], times[0]));
    else if(n_times > 1)
    {
        vector<int64_t> deltas;
        for (size_t i = 1; i < n_times; i++) {
            deltas.push_back(times[i] - times[i - 1]);
        }

        deque<int64_t> segment_start_indexes = {0};
        for(size_t di = 0; di < deltas.size(); di++)
        {
            if(deltas[di] > threshold)
                segment_start_indexes.push_back(di + 1);
        }

        while(!segment_start_indexes.empty())
        {
            int64_t start_index = segment_start_indexes.front();
            segment_start_indexes.pop_front();

            int64_t end_index = (segment_start_indexes.empty())?n_times-1:segment_start_indexes.front() - 1;

            segments.push_back(make_pair(times[start_index], times[end_index]));
        }
    }
#else
    int64_t start_ts = -1;
    int64_t last_ts = -1;

    while(!timesd.empty())
    {
        auto ts = timesd.front();

        auto n_remaining = timesd.size();

        if(start_ts == -1)
        {
            start_ts = ts;
            last_ts = -1;

            timesd.pop_front();

            if(n_remaining == 1)
                segments.push_back(make_pair(start_ts, ts));
        }
        else
        {
            if(last_ts != -1)
            {
                if(ts - last_ts > threshold)
                {
                    segments.push_back(make_pair(start_ts, last_ts));
                    start_ts = -1;
                    last_ts = -1;
                    continue;
                }
                else
                {
                    timesd.pop_front();

                    if(n_remaining == 1)
                        segments.push_back(make_pair(start_ts, ts));
                }
            }
        }

        last_ts = ts;
    }
#endif

    return segments;
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

            auto segments = find_contiguous_segments(kfst);

            string seg_str;
            for(auto s : segments)
                seg_str += "[" + r_string_utils::int64_to_s((s.second-s.first)/1000) + "], ";

            printf("camera: id=%s, bytes_per_second=%u, %s\n", s.camera.id.c_str(), s.bytes_per_second, seg_str.c_str());
        }
    }
    catch(exception& ex)
    {
        R_LOG_ERROR("exception: %s", ex.what());
        printf("exception: %s\n", ex.what());
    }
}
