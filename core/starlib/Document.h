#pragma once
#include "Starlab.h"
#include "Model.h"
#include "StarlabException.h"
#include "starlib_global.h"

namespace Starlab{

/// @brief The scene graph containing the models/layers
/// @ingroup starcore
class STARLIB_EXPORT Document : public QObject{
    Q_OBJECT 
    
/// @{ Busy system management
private:
    int _isBusy;     /// defaults 0
public:
    void pushBusy(); ///< Increases the busy level of the document 
    void popBusy();  ///< Decreases the busy status, emits hasChanged() if !isBusy
    bool isBusy();   /// Is the document busy? (i.e. busy during I/O)   
/// @}

/// @{ access to models
private:
    /// The list of all models in the document
    QList<Model*> _models;     
public:
    /// The models in this layer (the connection is a copy)
    QList<Model*> models(){ return _models; }
/// @} 
    
public:
    /// The name of the document, used as label
    QString name;
    /// Filepath where the document is saved (or will be...)
    QString path;    
    /// Has document been modified? (i.e. added/removed a model)
    bool isModified;
    /// Viewpoint transformation (identifies trackball)
    /// @todo use the QGLViewer transformation datatype
    // QMatrix4x4 transform;

/// @{ Selection management
private:
    /// the model which is currently selected
    Model* _selectedModel;
public:
    /// Retrieves the selection, exception thrown with empty selection
    Model* selectedModel();
    /// Updates the selection, making sure this is a valid model
    void setSelectedModel(Model* model);
    /// @todo Allow for multiple Selection
    // QList<Model*> selectedModels();
/// @}

/// @{ changes to model list
    void raise_layer(Model* model);
    void lower_layer(Model* model);
/// @}
    
public:
    /// Basic document constructor
    Document();
    /// Compute bounding box of all the elements in the scene
    BBox3 bbox();
    /// Add a model (if it's only one becomes selection)
    void addModel(Model* m);
    /// Remove a model, updates selection if necessary
    void deleteModel(Model* m);
    /// Return a model by name
    Model * getModel(QString name);
    /// Clean slate, remove all models
    void clear();
    /// Is there any model?
    bool isEmpty(){ return _models.empty(); }
    
    
    
/// @{ Document status updates
signals:
    void resetViewport();                ///< Document requesting a reset of the view matrixes
    void hasChanged();                   ///< Document has changed, emitted after "last" popBusy()
    void selectionChanged(Model* model); ///< The selection has changed to the provided model
public:
    void emit_resetViewport();
private:
    void emit_hasChanged();
/// @}
};

} // namespace

typedef Starlab::Document StarlabDocument;
