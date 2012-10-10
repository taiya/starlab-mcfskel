#pragma once
#include "CurveskelPlugins.h"
#include "CurveskelQForEach.h"

class curveskel_render_lines : public CurveskelRenderPlugin{
    Q_OBJECT
    Q_INTERFACES(RenderPlugin)

public: 
    RenderPlugin* factory(){ return new curveskel_render_lines(); }
    QString name() { return "Skeleton Edges"; }
    QIcon icon(){ return QIcon(":/sticks.png"); }
    void render();
    
    /// @{ OpenGL vertex buffers
    private:
        void render_nodes();
        void render_edges();
    private:
        std::vector<unsigned int>  triangles; ///< index array for buffered OpenGL rendering 
        std::vector<unsigned int>  edges;     ///< index array for buffered OpenGL rendering
    /// @}

protected:
    virtual bool isDefault() { return true; }
};
