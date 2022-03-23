#include "rtspcredentials.h"
#include "ui_rtspcredentials.h"

using namespace std;

RTSPCredentials::RTSPCredentials(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RTSPCredentials)
{
    _ui->setupUi(this);
    setWindowTitle("Camera Credentials");
    setModal(true);
}

RTSPCredentials::~RTSPCredentials()
{
    delete _ui;
}
