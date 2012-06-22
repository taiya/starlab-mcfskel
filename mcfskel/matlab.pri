MATLAB = $$(MATLAB)
isEmpty(MATLAB): error("MATLAB should point to /Applications/matlab.app")

INCLUDEPATH += $$MATLAB/extern/include/

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
