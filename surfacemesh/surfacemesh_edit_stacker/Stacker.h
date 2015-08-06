#pragma once
#include "interfaces/EditPlugin.h"
#include "StackerWidget.h"

class Stacker : public EditPlugin{
    Q_OBJECT
    Q_INTERFACES(EditPlugin)
  
/// Functions part of the EditPlugin system
public:   
    virtual void createEdit(Document* /*doc*/, StarlabDrawArea* /*parent*/){ widget=new StackerWidget(); }
    virtual void destroyEdit(Document* /*doc*/, StarlabDrawArea* /*parent*/){ delete widget; }    
};
