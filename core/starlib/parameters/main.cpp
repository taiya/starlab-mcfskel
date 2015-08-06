#include <QDebug>
#include <QApplication>
#include <QMainWindow>
#include "RichParameterSet.h"
#include "RichParameterWidget.h"
#include "ParametersFrame.h"

#include "RichFloat.h"
#include "RichBool.h"
#include "RichString.h"
#include "RichInt.h"
// #include "RichColor.h"
// #include "RichAbsPerc.h"

#include "RichPar.h"

int test_simple();
int test_subparset();
int test_subsubparset();
int test_parameterwidget(QFrame& f);
int test_richparametersframe();
int test_richpar();

int main(int argc, char *argv[]){
    QApplication app(argc, argv);
    
    int retval = 0;
    {
        retval |= test_richpar();
        // retval |= test_simple();
        // retval |= test_subparset();
        // retval |= test_subsubparset();
        // retval |= test_parameterwidget(f);
        // retval |= test_richparametersframe();
    }
    return retval;
}

int test_richpar(){
    RichPar::String stringPar;
    QVariant var;
    // Set the variant
    var.setValue(stringPar); 
    // Retrieve the variant
    RichPar::String s = var.value<RichPar::String>();
    
}

int test_richparametersframe(){
    ParametersFrame f;    
    RichParameterSet p;
    p.addParam(new RichFloat ("RichFloatExample1", 100.2f));
    p.addParam(new RichFloat ("RichFloatExample2", 100.3f));
    p.addParam(new RichInt   ("RichIntExample1", 100));
    p.addParam(new RichInt   ("RichIntExample2", 23));
    p.addParam(new RichBool  ("RichBoolExample1",  1.0f));
    p.addParam(new RichBool  ("RichBoolExample2",  2.0f));
    p.addParam(new RichString("RichStringExample1","something1"));
    p.addParam(new RichString("RichStringExample2","something2"));
    f.loadFrameContent(p);
    return QApplication::exec(); //Starts the GUIs
}

int test_parameterwidget(QFrame& f){ 
    RichFloat::Widget(&f, new RichFloat("RichFloatExample", 0.0));   
    // BoolWidget(&f, new RichBool("RichBoolExample", false));
    return 0;    
}
#if 0
int test_simple(){
    RichParameterSet p;
    p.addParam(new RichBool("RichBoolExample", false));
    p.addParam(new RichFloat("RichFloatExample", 0.0f));
    p.addParam(new RichInt("RichIntExample", 0));
    p.addParam(new RichString("RichStringExample","something"));
    p.addParam(new RichColor("RichColorExample",Qt::black));
    p.addParam(new RichAbsPerc("RichAbsPercExample",.5));
    p.addParam(new RichFilePath("RichFilePathExample","pathexample"));   
    qDebug() << qPrintable( p.toString() );
    return 0;
}
int test_subsubparset(){
    RichParameterSet p("PARAMETERSROOT");
    p.addParam(new RichBool("LV1_BOOL", false));
    p.addParam(new RichFloat("LV1_FLOAT", 0.0f));
    { 
        RichParameterSet* subpar = new RichParameterSet("LV1_GROUP");
        subpar->addParam(new RichBool("LV2_BOOL", false));
        subpar->addParam(new RichFloat("LV2_FLOAT", 0.0f));
        p.addParam(subpar);
        {
            RichParameterSet* subsubpar = new RichParameterSet("LV2_GROUP");
            subsubpar->addParam(new RichBool("LV3_BOOL", false));
            subsubpar->addParam(new RichFloat("LV3_FLOAT", 0.0f));            
            subpar->addParam(subsubpar);
        }
    }   
    qDebug() << qPrintable(p.toString());    
    return 0;
}
int test_subparset(){
    RichParameterSet p("PARAMETERSROOT");
    p.addParam(new RichBool("LV1_BOOL", false));
    p.addParam(new RichFloat("LV1_FLOAT", 0.0f));
    { 
        RichParameterSet* subpar = new RichParameterSet("LV1_GROUP1");
        subpar->addParam(new RichBool("LV2_BOOL", false));
        subpar->addParam(new RichFloat("LV2_FLOAT", 0.0f));
        p.addParam(subpar);
    }
    { 
        RichParameterSet* subpar = new RichParameterSet("LV1_GROUP2");
        subpar->addParam(new RichBool("LV2_BOOL", false));
        subpar->addParam(new RichFloat("LV2_FLOAT", 0.0f));
        p.addParam(subpar);
    }
    qDebug() << qPrintable(p.toString());    
    return 0;
}
#endif
