#include <stdio.h>
#include <Python.h>
// #include "pyhelper.hpp"
// #include <pyhelper.hpp>

//gcc -I/usr/include/python3.10 -lpython3.10 test_call_py_from_c.cpp  -o test_call_py_from_c

int main(){

    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"/lib64/security/keystrokes_models/\")");
    
    PyObject* myModuleString = PyUnicode_DecodeFSDefault((char*)"keystrokes_authentication");
    PyObject* myModule = PyImport_Import(myModuleString);
    if(myModule == NULL){
        printf("%s\n", "here1");
        return 0;
    }
    PyObject* myFunction = PyObject_GetAttrString(myModule,(char*)"authenticate_keystrokes");

    if(myFunction == NULL){
        printf("%s\n", "here2");
        return 0;
    }
    PyObject* args = PyTuple_Pack(1, PyUnicode_DecodeFSDefault((char*)"katy"));
    // Py_XDECREF(myFunction);
    PyObject* myResult = PyObject_CallObject(myFunction, args);
    double result = PyFloat_AsDouble(myResult);
    Py_Finalize();

    // printf("%.2f\n", result);



    // CPyObject myModuleString = PyUnicode_FromString("authenticate_keystrokes");
    // CPyObject myModule = PyImport_Import(myModuleString);
    // PyObject* myFunction = PyObject_GetAttrString(myModule,(char*)"authenticate_keystrokes");
    // PyObject* args = PyTuple_Pack(1,PyFloat_FromDouble(2.0));
    // PyObject* myResult = PyObject_CallObject(myFunction, NULL);
    // double result = PyFloat_AsDouble(myResult);
    // printf("%f", result);
    return 0;
}