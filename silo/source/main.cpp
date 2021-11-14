#include "mainwindow.h"
#include "r_utils/r_args.h"
#include "r_utils/r_logger.h"
#include "r_storage/r_storage_file.h"
#include <string>
#include <QApplication>

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
    Q_INIT_RESOURCE(silo);

    R_LOG_INFO("Revere Silo Starting...");

    auto arguments = r_args::parse_arguments(argc, argv);

    string storage_file_path;
    if(r_args::check_argument(arguments, "--allocate_storage_file", storage_file_path))
    {
        auto n_blocks = r_string_utils::s_to_uint32(r_args::get_required_argument(arguments, "--n_blocks"));
        auto block_size = r_string_utils::s_to_uint32(r_args::get_required_argument(arguments, "--block_size"));

        r_storage::r_storage_file::allocate(storage_file_path, block_size, n_blocks);

        return 0;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
