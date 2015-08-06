#pragma once
#include <QDockWidget>
class FilterPlugin;
class RichParameterSet;
class ParametersFrame;

namespace Ui { class FilterDockWidget; }
class FilterDockWidget : public QDockWidget{
    Q_OBJECT
    
public:
    explicit FilterDockWidget(FilterPlugin* filter, RichParameterSet* pars, QWidget *parent = 0);
    ~FilterDockWidget();
    
private slots:
    void on_applyButton_released();
    void on_defaultButton_pressed();
    void adjustSize();
    
public slots:
    void init(RichParameterSet* parameters);
signals:
    void execute(FilterPlugin* filter, RichParameterSet* parameters);    
private:
    Ui::FilterDockWidget *ui;
    FilterPlugin* filterPlugin;
    ParametersFrame* parFrame;
    RichParameterSet* pars;
};
