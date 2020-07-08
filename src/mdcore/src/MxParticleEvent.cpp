/*
 * MxParticleEvent.cpp
 *
 *  Created on: Jun 25, 2020
 *      Author: andy
 */

#include <MxParticleEvent.h>
#include <MxParticle.h>
#include <engine.h>
#include <space.h>
#include <iostream>

static HRESULT particletimeevent_pyfunction_invoke(CTimeEvent *event, double time);

static HRESULT particletimeevent_exponential_setnexttime(CTimeEvent *event, double time);

PyObject* MxOnTime(PyObject *module, PyObject *args, PyObject *kwargs)
{
    std::cout << MX_FUNCTION << std::endl;
    std::cout << "obj: " << PyUnicode_AsUTF8AndSize(PyObject_Str(module), NULL) << std::endl;
    std::cout << "args: " << PyUnicode_AsUTF8AndSize(PyObject_Str(args), NULL) << std::endl;
    std::cout << "kwargs: " << PyUnicode_AsUTF8AndSize(PyObject_Str(kwargs), NULL) << std::endl;
    
    MxParticleTimeEvent* event = (MxParticleTimeEvent*)CTimeEvent_New();
    
    if(CTimeEvent_Init(event, args, kwargs) != 0) {
        Py_DECREF(event);
        return NULL;
    }
    
    CMulticastTimeEvent_Add(_Engine.on_time, event);
    
    return event;
}


/**
 * static method to create an on_time event object
 */



MxParticleTimeEvent *MxParticleTimeEvent_New(PyObject *args, PyObject *kwargs) {
    
}

HRESULT _MxTimeEvent_Init(PyObject *m)
{
    return  S_OK;
}

HRESULT MxParticleTimeEvent_BindParticleMethod(CTimeEvent *event,
                                               struct MxParticleType *target, PyObject *method)
{
    
    std::cout << "target: " << PyUnicode_AsUTF8AndSize(PyObject_Str((PyObject*)target), NULL) << std::endl;
    std::cout << "method: " << PyUnicode_AsUTF8AndSize(PyObject_Str(method), NULL) << std::endl;
    
    
    return E_NOTIMPL;
}

HRESULT MxParticleType_BindEvent(MxParticleType *type, PyObject *e) {
    
    if(PySequence_Check(e)) {
        
    }
    
    else if(PyObject_IsInstance(e, (PyObject*)&CTimeEvent_Type)) {
        
        CTimeEvent *timeEvent = (CTimeEvent*)e;
        
        if(timeEvent->target) {
            return c_error(E_FAIL, "event target already set in particle type definition");
        }
        
        timeEvent->target = (PyObject*)type;
        Py_INCREF(timeEvent->target);
        
        timeEvent->next_time = 10;
        
        timeEvent->flags |= EVENT_ACTIVE;
        
        timeEvent->te_invoke = (timeevent_invoke)particletimeevent_pyfunction_invoke;
        
        if(timeEvent->flags & EVENT_EXPONENTIAL) {
            timeEvent->te_setnexttime = particletimeevent_exponential_setnexttime;
        }
    }
    
    return S_OK;
}

HRESULT MyParticleType_BindEvents(struct MxParticleType *type, PyObject *events)
{
    std::cout << "type: " << PyUnicode_AsUTF8AndSize(PyObject_Str((PyObject*)type), NULL) << std::endl;
    std::cout << "events: " << PyUnicode_AsUTF8AndSize(PyObject_Str(events), NULL) << std::endl;
    
    if (PySequence_Check(events) == 0) {
        return c_error(E_FAIL, "events must be a list");
    }
    
    for(int i = 0; i < PySequence_Size(events); ++i) {
        PyObject *e = PySequence_Fast_GET_ITEM(events, i);
        
        HRESULT r = MxParticleType_BindEvent(type, e);
        
        if(!SUCCEEDED(r)) {
            return r;
        }
    }
    
    return S_OK;
}


HRESULT particletimeevent_pyfunction_invoke(CTimeEvent *event, double time) {
    
    if(event->next_time > time) {
        return S_OK;
    }
    
    
    
    MxParticleType *type = (MxParticleType*)event->target;
    
    std::uniform_int_distribution<int> distribution(0,type->nr_parts-1);
    
    PyObject *args = PyTuple_New(2);
    
    int pid = distribution(CRandom);
    
    PyObject *t = PyFloat_FromDouble(time);
    PyTuple_SET_ITEM(args, 0, _Engine.s.partlist[pid]->pyparticle);
    PyTuple_SET_ITEM(args, 1, t);
    
    //std::cout << MX_FUNCTION << std::endl;
    //std::cout << "args: " << PyUnicode_AsUTF8AndSize(PyObject_Str(args), NULL) << std::endl;
    //std::cout << "method: " << PyUnicode_AsUTF8AndSize(PyObject_Str(event->method), NULL) << std::endl;
    
    // time expired, so invoke the event.
    PyObject *result = PyObject_CallObject((PyObject*)event->method, args);
    
    
    return S_OK;
}

PyObject *MxInvokeTime(PyObject *module, PyObject *args, PyObject *kwargs) {
    
    double time = PyFloat_AsDouble(PyTuple_GetItem(args, 0));
    
    CMulticastTimeEvent_Invoke(_Engine.on_time, time);
    
    
    Py_RETURN_NONE;
}

// need to scale period by number of particles.
HRESULT particletimeevent_exponential_setnexttime(CTimeEvent *event, double time) {
    MxParticleType *type = (MxParticleType*)event->target;
    std::exponential_distribution<> d(event->period / type->nr_parts);
    event->next_time = time + d(CRandom);
    return S_OK;
}