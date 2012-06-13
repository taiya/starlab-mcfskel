#pragma once
#include "interfaces/FilterPlugin.h"

class mcfskel : public SelectionFilterPlugin{
    Q_OBJECT
    Q_INTERFACES(FilterPlugin)

public:
    QString name() { return "mcfskel"; }
    QString description() { return "mcfskel description"; }
	void applyFilter(Model*,RichParameterSet*, StarlabDrawArea*);
};
