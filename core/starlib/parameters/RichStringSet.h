#pragma once
#pragma once

#include <QComboBox>
#include "RichParameter.h"
#include "RichParameterWidget.h"
#include "RichString.h"
#include <stdexcept>

/// Combo box of strings, Value inherited from RichString but widget 
/// is a combobox!!
class RichStringSet : public RichParameter{
private:
    QList<QString> entries;
    
public:
    virtual RichParameterWidget* getWidget(QWidget* parent){ 
        return new ComboBoxWidget(parent, this); 
    }
    RichStringSet(QString name, QList<QString> entries, QString description="", QString tooltip="")
        :RichParameter(name, description, tooltip){
        if(!entries.empty()) {
            this->entries = entries;
            this->defaultValue = new RichString::Value(entries.first());
            this->val = new RichString::Value(entries.first());
        }
    }
    
public:
    class ComboBoxWidget : public RichParameterWidget{
    protected:
        QLabel*    label;
        QComboBox* combo;
    public:
        ~ComboBoxWidget(){ delete label; delete combo; }
        ComboBoxWidget(QWidget* p,RichStringSet* richStringSet) : RichParameterWidget(p,richStringSet) {
            /// Allocate
            label = new QLabel(richStringSet->description,this);
            combo = new QComboBox(this);
            label->setAlignment(Qt::AlignRight);
            /// Fill the combo
            foreach(QString entry, richStringSet->entries)
                combo->addItem(entry);
            /// Add to layout
            gridLay->addWidget(label,row,0,Qt::AlignLeft);
            gridLay->addWidget(combo,row,1,Qt::AlignRight);            
        }
        void collectWidgetValue(){
            rp->val->set(RichString::Value(combo->currentText()));
        }
        /// @todo
        void resetWidgetValue(){ assert(0); }
        void setWidgetValue(const Value& /*nv*/){ assert(0); }
    };
};
