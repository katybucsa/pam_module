// #include<iostream>
// #include <stdio.h>
#include <Python.h>
#include <fstream>
#include <security/pam_modules.h>

#include "util.h"

int authenticate_keystrokes(std::string username)
{
    // Py_Initialize();
    // PyRun_SimpleString("import sys");
    // PyRun_SimpleString("sys.path.append(\"/lib64/security/keystrokes_models/\")");
    // PyRun_SimpleString("import os");
    // Py_SetPythonHome(L"/usr/bin/python3");
    // Py_SetPath(L"/usr/local/lib/python3.10");

    // PyObject* myModuleString = PyUnicode_FromString("abc");
    // PyObject* myModule = PyImport_Import(myModuleString);
    // PyObject* myFunction = PyObject_GetAttrString(myModule, (char*)"myf");
    //  if( myFunction == NULL){
    //     std::cout << "\nhere\n";
    //     return PAM_SUCCESS;
    // }
    // PyObject* myResult = PyObject_CallObject(myFunction, NULL);
    // int result = (int)PyFloat_AsDouble(myResult);
    // PyObject* myModuleString = PyUnicode_FromString("authenticate_keystrokes");
    // printf("5");
    // PyObject* myModule = PyImport_ImportModule("authenticate_keystrokes");
    // if (myModule == NULL)
    // {
    //     std::cout << "\nhere\n";
    // }
    
    // printf("6");
    // PyObject* myFunction = PyObject_GetAttrString(myModule, "authenticate_keystrokes_func");
    // // printf("7");
    // PyObject* myResult = PyObject_CallObject(myFunction, NULL);
    // // printf("8");
    // int result = (int)PyFloat_AsDouble(myResult);
    // // printf("9");
    // Py_Finalize();
    // printf("\n%d\n", result);

    std::ifstream file(pred_path(username));
    std::string line;
    getline (file, line);
    if(line.empty())
        return PAM_AUTH_ERR;
    int result = stoi(line);

    // Py_Initialize();
    // PyRun_SimpleString("import sys");
    // PyRun_SimpleString("sys.path.append(\"/lib64/security/keystrokes_models/\")");
    
    // PyObject* myModuleString = PyUnicode_DecodeFSDefault((char*)"keystrokes_authentication");
    // PyObject* myModule = PyImport_Import(myModuleString);
    // if(myModule == NULL){
    //     printf("%s\n", "here1");
    //     return 0;
    // }
    // PyObject* myFunction = PyObject_GetAttrString(myModule,(char*)"authenticate_keystrokes");

    // if(myFunction == NULL){
    //     printf("%s\n", "here2");
    //     return 0;
    // }
    // PyObject* args = Py_BuildValue("s", PyUnicode_DecodeFSDefault((char*)"katy"));
    // // Py_XDECREF(myFunction);
    // PyObject* myResult = PyObject_CallObject(myFunction, args);
    // double result = PyFloat_AsDouble(myResult);
    // Py_Finalize();

    if (result == 0)
        return PAM_AUTH_ERR;
    return PAM_SUCCESS;
}