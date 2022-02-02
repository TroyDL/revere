#include "friendlyname.h"
#include "ui_friendlyname.h"

using namespace std;

FriendlyName::FriendlyName(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FriendlyName)
{
    _ui->setupUi(this);
    setWindowTitle("Camera Name");
    setModal(true);
}

FriendlyName::~FriendlyName()
{
    delete _ui;
}
