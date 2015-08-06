#pragma once
#include <QMessageBox>
class OpenGLErrorChecker{
private:
    QString message;
    bool hasErrors;
public:
    OpenGLErrorChecker(QString location){
        hasErrors = false;
        message = "OpenGL errors at: " +location+ "\n"; 
        int errorcode = glGetError();
        while(errorcode != GL_NO_ERROR){
            hasErrors = true;
            switch(errorcode){
                case GL_INVALID_ENUM:				message+=("  invalid enum");		break;				
                case GL_INVALID_VALUE:				message+=("  invalid value");		break;
                case GL_INVALID_OPERATION:          message+=("  invalid operation");	break;
                case GL_STACK_OVERFLOW:				message+=("  stack overflow");		break;
                case GL_STACK_UNDERFLOW:			message+=("  stack underflow");		break;				
                case GL_OUT_OF_MEMORY:				message+=("  out of memory");		break;
                case GL_TABLE_TOO_LARGE:            message+=("  table too large");		break;
                default:                            message+=("  unknown");             break;
            }
            #ifndef EXTRACT_ALL_ERRORS
            break; 
            #endif
            errorcode = glGetError(); 
        }
        
        /// Shows errors
        if(hasErrors) 
            qDebug() << message;
    }

#if 0
	static void qDebug(const char* m) {
		QString message=makeString(m);
        if(message.isEmpty()) return;
        ::qDebug("%s",qPrintable(message));
	}	
	
	static void QMessageBox(const char* m, const char* title) {
		QString message=makeString(m);
		QMessageBox::warning(0, title,message);
	}	
	static void QMessageBox(const char* m) {QMessageBox(m,"GL error");};
#endif
};
