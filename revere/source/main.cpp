#include "mainwindow.h"
#include "r_utils/r_args.h"
#include "r_utils/r_logger.h"
#include "r_storage/r_storage_file.h"
#include "r_onvif/r_onvif_session.h"
#include <string>
#include <QApplication>
#include <algorithm>

//
// NOTE
// Adding a QPushButton to a table cell:
//
//    QWidget* pWidget = new QWidget();
//    QPushButton* btn_edit = new QPushButton();
//    btn_edit->setText("Edit");
//    QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
//    pLayout->addWidget(btn_edit);
//    pLayout->setAlignment(Qt::AlignCenter);
//    pLayout->setContentsMargins(0, 0, 0, 0);
//    pWidget->setLayout(pLayout);
//    ui.table->setCellWidget(i, 3, pWidget);
//

using namespace r_utils;
using namespace std;

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(revere);

    R_LOG_INFO("Revere Starting...");

    auto arguments = r_args::parse_arguments(argc, argv);

    string storage_file_path;
    if(r_args::check_argument(arguments, "--allocate_storage_file", storage_file_path))
    {
        auto n_blocks = r_string_utils::s_to_uint32(r_args::get_required_argument(arguments, "--n_blocks"));
        auto block_size = r_string_utils::s_to_uint32(r_args::get_required_argument(arguments, "--block_size"));

        r_storage::r_storage_file::allocate(storage_file_path, block_size, n_blocks);

        return 0;
    }

    if(r_args::check_argument(arguments, "--discover"))
    {
        r_onvif::r_onvif_session session;
        auto devices = session.discover();

        for(auto& d : devices)
        {
            printf("camera_name: %s, ipv4: %s\n",d.camera_name.c_str(), d.ipv4.c_str());
        }

        return 0;
    }

    auto camera_ipv4 = r_args::get_optional_argument(arguments, "--get_camera_info_by_ipv4");

    if(!camera_ipv4.is_null())
    {
        r_onvif::r_onvif_session session;
        auto devices = session.discover();

        auto found = find_if(begin(devices), end(devices), [&] (const r_onvif::r_onvif_discovery_info& d) { return d.ipv4 == camera_ipv4.value(); });

        if(found != end(devices))
        {
            auto info = *found;

            auto username = r_args::get_optional_argument(arguments, "--username");
            auto password = r_args::get_optional_argument(arguments, "--password");

            auto ci = session.get_rtsp_url(info, username, password);

            if(!ci.is_null())
            {
                printf("camera_name: %s\n", ci.value().camera_name.c_str());
                printf("address: %s\n", ci.value().address.c_str());
                printf("serial_number: %s\n", ci.value().serial_number.c_str());
                printf("model_number: %s\n", ci.value().model_number.c_str());
                printf("firmware_version: %s\n", ci.value().firmware_version.c_str());
                printf("manufacturer: %s\n", ci.value().manufacturer.c_str());
                printf("hardware_id: %s\n", ci.value().hardware_id.c_str());
                printf("rtsp_url: %s\n", ci.value().rtsp_url.c_str());
            }
        }
        else
        {
            printf("ipv4 not found\n");
        }

        return 0;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
