#pragma once
#include "ProjectInputOutputPlugin.h"

class project_io_starlab : public ProjectInputOutputPlugin{
    Q_OBJECT
    Q_INTERFACES(ProjectInputOutputPlugin)

public:
    QString name() { return "Starlab Project (*.starlab)"; }
    virtual bool open(const QString& path, Starlab::Application& app);
    virtual void save(const QString& path, Starlab::Document& m);
};
