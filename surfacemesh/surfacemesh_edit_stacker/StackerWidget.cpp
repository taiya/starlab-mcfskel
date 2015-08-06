#include "Stacker.h"
#include "ui_StackerWidget.h"
#include <QDebug>

StackerWidget::StackerWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

StackerWidget::~StackerWidget(){
    qDebug()<<"Widget::~Widget()";
    delete ui;
}
