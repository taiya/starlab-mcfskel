CONFIG += starlab
STARLAB_TEMPLATE += plugin 
STARLAB_DEPENDS += surfacemesh_model

DEPENDPATH  *= $$PWD
INCLUDEPATH *= $$PWD

MATLAB = $$(MATLAB)
isEmpty(MATLAB): error("MATLAB should point to /Applications/matlab.app")
macx{
    DYLD_LIBRARY_PATH = $$(DYLD_LIBRARY_PATH)
    isEmpty(DYLD_LIBRARY_PATH): error("DYLD_LIBRARY_PATH should point to /Applications/matlab.app/bin/maci64")
    LIBS += -L$$(DYLD_LIBRARY_PATH)
    LIBS += -leng -lmex -lmat -lmx -lut
}

win32{
    MATLAB_LIBS = "$$(MATLAB)extern\\lib\\win32\\microsoft"
    LIBS += $$MATLAB_LIBS\\libeng.lib $$MATLAB_LIBS\\libmex.lib \
            $$MATLAB_LIBS\\libmat.lib $$MATLAB_LIBS\\libmx.lib $$MATLAB_LIBS\\libut.lib
}
INCLUDEPATH += $$(MATLAB)/extern/include/

HEADERS += \
    Skelcollapse.h \
    SurfaceAreaHelper.h \
    MatlabLaplacianHelper.h \
    LegacyLaplacianHelper.h \
    aux_medial_contraction.cpp \
    TopologyJanitor.h

SOURCES += \
    Skelcollapse.cpp


