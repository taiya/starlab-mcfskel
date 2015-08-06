#include "CmdLineParser.h"
CmdLineParser::CmdLineParser(int argc, char *argv[], QObject *parent){
    this->setParent(parent);
    parser.setArguments(argc, argv);
    
    /// Init
    listFilters = false;
    saveOverwrite = false;
    saveCreatecopy = false;
    executeFilter = "";
    showExamples = false;
    noArguments = true;
    
    /// Real options
    parser.enableVersion(true); ///< enable -v // --version
    parser.enableHelp(true); ///< enable -h / --help
    parser.addParam("model", "The path of model files to be processed", QCommandLine::Multiple);
    
    /// Filter options 
    parser.addSwitch(QChar::Null, "examples",       "Show usage examples", QCommandLine::Optional);
    parser.addSwitch(QChar::Null, "list-filters",   "Shows available filters");
    parser.addOption(QChar::Null, "filter",         "Runs the specified filter, to show available filters type \"starlab --show-filters\"", QCommandLine::Multiple);
    parser.addSwitch(QChar::Null, "save",           "Save the filtered models by creating a new copy", QCommandLine::Optional);
    parser.addSwitch(QChar::Null, "save-overwrite", "Overwrites the models after they have been filtered", QCommandLine::Optional);
        
    /// Set this class as the parser
    connect(&parser, SIGNAL(switchFound(const QString &)), this, SLOT(switchFound(const QString &)));
    connect(&parser, SIGNAL(optionFound(const QString &, const QVariant &)), this, SLOT(optionFound(const QString &, const QVariant &)));
    connect(&parser, SIGNAL(paramFound(const QString &, const QVariant &)), this, SLOT(paramFound(const QString &, const QVariant &)));
    connect(&parser, SIGNAL(parseError(const QString &)), this, SLOT(parseError(const QString &)));
    
    /// Default hardcoded options
    parser.parse();
    
    /// Show help when nothing (aside from Qt options) was given
    if(noArguments) parser.showHelp(true,0);
}

void CmdLineParser::parseError(const QString& error){
    qWarning() << qPrintable(error);
    parser.showHelp(true, -1);
    exit(-1);
}

/// i.e. --help, -h, --list-filters
void CmdLineParser::switchFound(const QString & name){
    // qWarning() << "Switch:" << name << name.toString();
    noArguments=false;
    if(name=="list-filters") listFilters = true;
    if(name=="save") saveCreatecopy = true;
    if(name=="save-overwrite") saveOverwrite = true;
    if(name=="examples") showExamples = true;
}

/// Name/Value pair: i.e. --filter="something" means optionFound("filter","something")
void CmdLineParser::optionFound(const QString & name, const QVariant & value){
    // qWarning() << "Option:" << name << value.toString();
    noArguments=false;
    if(name=="filter") executeFilter = value.toString();
}

/// Input (everything that is not option) i.e. ~/Data/mesh.off 
void CmdLineParser::paramFound(const QString& /*name*/, const QVariant & value){
    // qWarning() << "Parameter:" << name << value.toString();
    noArguments=false;
    inputModels.append(value.toString());
}

