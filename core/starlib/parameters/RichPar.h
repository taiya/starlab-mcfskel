#pragma once
#include <QVariant>

class RichPar{
    /// @Note: QVariant requires me to define these for each subtype
    //    Base(const Base& ){}
    //    Base();
    //    ~Base(){}
    
public:
    class Base{
        
    };

    class Int{
        
    };
    class Bool{
        
    };
    class Float{
        
    };
    class String{
        
    };
};

Q_DECLARE_METATYPE ( RichPar::Int )
Q_DECLARE_METATYPE ( RichPar::Bool )
Q_DECLARE_METATYPE ( RichPar::Float )
Q_DECLARE_METATYPE ( RichPar::String )
