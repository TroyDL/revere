#include "newfilename.h"
#include "ui_newfilename.h"

using namespace std;

NewFileName::NewFileName(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::NewFileName)
{
    _ui->setupUi(this);
    setModal(true);
}

NewFileName::~NewFileName()
{
    delete _ui;
}
