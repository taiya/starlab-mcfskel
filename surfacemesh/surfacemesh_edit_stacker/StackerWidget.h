#pragma once
#include <QDockWidget>
#include <QDebug>

namespace Ui {
    class Widget;
}

class StackerWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit StackerWidget(QWidget *parent = 0);
    ~StackerWidget();
   
private:
    Ui::Widget *ui;
};
