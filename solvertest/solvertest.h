#pragma once
#include "interfaces/FilterPlugin.h"

class solvertest : public SelectionFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "solvertest"; }
    QString description() { return "solvertest description"; }
	void applyFilter(Model*,RichParameterSet*, StarlabDrawArea*);
};
