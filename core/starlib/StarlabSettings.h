#pragma once
#include <QSettings>
#include <QDebug>
#include <QColor>

#include "starlib_global.h"
#include "StarlabApplication.h"

namespace Starlab{

/** @brief Global settings management class. The settings saved within this system
 * are automatically synced with a settings.ini file
 *
 * @todo precise location of the file
 * @ingroup starcore
 */
class STARLIB_EXPORT Settings{
    
private: 
    QSettings* qsettings;
    Application* const _application;
public:
    /**
      * The constructor first set some hardcoded parameters, 
      * then attempts to override their values with the ones
      * specified by the xml options file 
      */
    Settings(Application *application);
    void sync(){ qsettings->sync(); }
    
    /// Sets hardcoded default setting value (writes only if value is not already defined)
    void setDefault(const QString& key, const QVariant& value);
    void set(const QString& key, const QVariant& value);

    QVariant get(const QString& key);
    bool contains(const QString& key);
    void checkContains(const QString& key);
    
    /// Simplified / Specialized converters
    /// @{ @name Type-Specialized getters    
    public:
        bool getBool(const QString& key);
        float getFloat(const QString& key);
        float* getColor4fv(const QString& key);     
        QColor getQColor(const QString& key);
        QString getString(const QString& key);
        QStringList getStringList(const QString& key);
    /// @}

        
    /// @{ Settings I/O
        /// Where will the "ini" file be loaded/saved from?
        /// If the execution path contains a starlab.ini file we will it.
        /// Otherwise we use the one stored at:
        /// - Win: $$STARLAB/starlab.ini
        /// - Mac: $$STARLAB/starlab.ini
        /// - Linux: ~/.starlab
        QString settingsFilePath();
    
        /// @todo load settings from file & hooks in GUI
        // void load(const QString& filename="");
        
        /// @todo save settings to file & hooks in GUI
        // void save(const QString& filename="");
    /// @}
    
    /// Print to string (e.g. qDebug() << this->toString() )
    QByteArray toLocal8Bit();
};

} // namespace
