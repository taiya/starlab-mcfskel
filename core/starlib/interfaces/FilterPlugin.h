#pragma once
#include "Model.h"
#include "StarlabPlugin.h"

class STARLIB_EXPORT FilterPlugin : public StarlabPlugin{
public:
    virtual void applyFilter(RichParameterSet*) = 0;
    virtual void initParameters(RichParameterSet*){}
    virtual bool isApplicable(Starlab::Model* model) = 0;

    /// @{ access to resources
    protected:
        using StarlabPlugin::document;
        using StarlabPlugin::drawArea;
        Starlab::Model* model(){ return document()->selectedModel(); }    
    /// @}
};

Q_DECLARE_INTERFACE(FilterPlugin, "Starlab::FilterPlugin/2.0")
