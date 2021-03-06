/*
 * Copyright (C) 2016 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @controller_pythonEmbedded.h
 * The PythonEmbedded class, which runs a Python 3.x controller
 */

#ifndef SPLASH_PYTHON_EMBEDDED_H
#define SPLASH_PYTHON_EMBEDDED_H

#include <atomic>
#include <future>
#include <iostream>
#include <string>
#include <thread>

#include <Python.h>

#include "./attribute.h"
#include "./controller.h"
#include "./coretypes.h"
#include "./filter.h"
#include "./sink.h"

namespace Splash
{

/*************/
class PythonEmbedded : public ControllerObject
{
  public:
    /**
     * \brief Constructor
     * \param root Root object
     */
    PythonEmbedded(RootObject* root);

    /**
     * \brief Destructor
     */
    ~PythonEmbedded() final;

    /**
     * \brief Set the path to the source Python file
     * \param filepath Path to the source file
     * \return Return true if the file exists
     */
    bool setScriptFile(const std::string& filepath);

    /**
     * \brief Run the script
     * \return Return true if the script launched successfully
     */
    bool run();

    /**
     * \brief Stop the running script
     */
    void stop();

  private:
    std::string _filepath{""};   //!< Path to the python script
    std::string _scriptName{""}; //!< Name of the module (filename minus .py)
    Values _pythonArgs{};        //!< Command line arguments to send to the scripts

    bool _doLoop{false};                     //!< Set to false to stop the Python loop
    int _updateRate{200};                    //!< Loops per second
    std::thread _loopThread{};               //!< Python thread loop
    std::promise<bool> _loopThreadPromise{}; //!< Holds the output result from the threading loop

    PyObject* _pythonModule{nullptr};                //!< Loaded module (from the specified script)
    PyThreadState* _pythonLocalThreadState{nullptr}; //!< Local Python thread state, for the sub-interpreter

    std::mutex _attributeCallbackMutex{};
    std::map<uint32_t, CallbackHandle> _attributeCallbackHandles{};

    /**
     * \brief Python interpreter main loop
     */
    void loop();

    /**
     * \brief Get a Python function from the given module
     * \param module Python module
     * \param name Function name
     * \return Return a python function, or nullptr if it does not exist
     */
    PyObject* getFuncFromModule(PyObject* module, const std::string& name);

    /**
     * \brief Build a Python object from a Value
     * \param value Value to convert
     * \param toDict Convert Values to a dict if true
     * \return Return a PyObject* representation of the value
     */
    static PyObject* convertFromValue(const Value& value, bool toDict = false);

    /**
     * \brief Build a Value from a valid Python object
     * \param pyObject Python object to interpret
     * \return Return a Value
     */
    static Value convertToValue(PyObject* pyObject);

    /**
     * \brief Register new functors to modify attributes
     */
    void registerAttributes();

  private:
    static std::atomic_int _pythonInstances;        //!< Number of Python scripts running
    static PyThreadState* _pythonGlobalThreadState; //!< Global Python thread state, shared by all PythonEmbedded instances

    // Python objects and methods
    static PyMethodDef SplashMethods[];
    static PyModuleDef SplashModule;
    static PyObject* SplashError;

    static PyObject* pythonInitSplash();
    static PythonEmbedded* getSplashInstance();
    static PyObject* pythonGetInterpreterName(PyObject* self, PyObject* args);
    static PyObject* pythonGetLogs(PyObject* self, PyObject* args);
    static PyObject* pythonGetTimings(PyObject* self, PyObject* args);
    static PyObject* pythonGetMasterClock(PyObject* self, PyObject* args);
    static PyObject* pythonGetObjectList(PyObject* self, PyObject* args);
    static PyObject* pythonGetObjectTypes(PyObject* self, PyObject* args);
    static PyObject* pythonGetObjectDescription(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectAttributeDescription(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectType(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectsOfType(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectAttribute(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectAttributes(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonGetObjectLinks(PyObject* self, PyObject* args);
    static PyObject* pythonGetObjectReversedLinks(PyObject* self, PyObject* args);
    static PyObject* pythonGetTypesFromCategory(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSetGlobal(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSetObject(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSetObjectsOfType(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonAddCustomAttribute(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonRegisterAttributeCallback(PyObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonUnregisterAttributeCallback(PyObject* self, PyObject* args, PyObject* kwds);

    // Sink wrapper-specific stuff
  public:
    static std::atomic_int sinkIndex;
    typedef struct
    {
        bool initialized{false};
        PyObject_HEAD std::string sourceName{""};
        uint32_t width{512};
        uint32_t height{512};
        bool keepRatio{false};
        uint32_t framerate{30};
        std::string sinkName{""};
        std::string filterName{""};
        std::shared_ptr<Splash::Sink> sink{nullptr};
        bool linked{false};
        bool opened{false};
        PyObject* lastBuffer{nullptr};
    } pythonSinkObject;

    // Sink wrapper methods. They are in this class to be able to access the Splash capsule
    static void pythonSinkDealloc(pythonSinkObject* self);
    static PyObject* pythonSinkNew(PyTypeObject* type, PyObject* args, PyObject* kwds);
    static int pythonSinkInit(pythonSinkObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSinkLink(pythonSinkObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSinkUnlink(pythonSinkObject* self);
    static PyObject* pythonSinkGrab(pythonSinkObject* self);
    static PyObject* pythonSinkSetSize(pythonSinkObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSinkGetSize(pythonSinkObject* self);
    static PyObject* pythonSinkKeepRatio(pythonSinkObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSinkSetFramerate(pythonSinkObject* self, PyObject* args, PyObject* kwds);
    static PyObject* pythonSinkOpen(pythonSinkObject* self);
    static PyObject* pythonSinkClose(pythonSinkObject* self);
    static PyObject* pythonSinkGetCaps(pythonSinkObject* self);

    static PyMethodDef SinkMethods[];

  private:
    static PyTypeObject pythonSinkType;
};

} // end of namespace

#endif // SPLASH_PYTHON_EMBEDDED_H
