#pragma once 
class FileValue : public Value {
public:
    FileValue(QString filename) :pval(filename) {}
    inline QString getFileName() const {return pval;}
    inline bool isFileName() const {return true;}
    inline QString typeName() const {return QString("FileName");}
    inline void	set(const Value& p) {pval = p.getFileName();}
    QString toString() { return getFileName(); }    
    ~FileValue() {}
private:
    QString pval;
};
