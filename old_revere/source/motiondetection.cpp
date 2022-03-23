#include "motiondetection.h"
#include "ui_motiondetection.h"

using namespace std;

MotionDetection::MotionDetection(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::MotionDetection)
{
    _ui->setupUi(this);
    setModal(true);
}

MotionDetection::~MotionDetection()
{
    delete _ui;
}
