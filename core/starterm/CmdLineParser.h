#pragma once
#include <QDebug>
#include <QCoreApplication>
#include <QVariant>
#include "QCommandLine.h"

class CmdLineParser : public QObject{
    Q_OBJECT

public:
    CmdLineParser(int argc, char *argv[], QObject* parent);

public:
    bool listFilters;
    bool saveOverwrite;
    bool saveCreatecopy;
    bool showExamples;
    bool noArguments;
    QString executeFilter;
    QStringList inputModels;

private:
    QCommandLine parser;
    
private slots:
    /// Errors result in app termination
    void parseError(const QString & name);
    void switchFound(const QString & name);
    void optionFound(const QString & name, const QVariant & value);
    void paramFound(const QString & name, const QVariant & value);
};
