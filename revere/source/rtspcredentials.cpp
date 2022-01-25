#include "rtspcredentials.h"
#include "ui_rtspcredentials.h"

using namespace std;

RTSPCredentials::RTSPCredentials(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::RTSPCredentials)
{
    _ui->setupUi(this);
}

RTSPCredentials::~RTSPCredentials()
{
    delete _ui;
}
