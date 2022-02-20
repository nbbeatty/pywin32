// This file implements the IMAPIAdviseSink Interface and Gateway for Python.
// Generated by makegw.py

#include "PythonCOM.h"
#include "PythonCOMServer.h"
#include "PyMAPIUtil.h"
#include "PyIMAPIAdviseSink.h"

#ifdef __MINGW32__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
PyObject *PyObject_FromNOTIFICATION(NOTIFICATION *n)
{
    PyObject *ret = NULL;
    switch (n->ulEventType) {
        case fnevCriticalError: {
            ERROR_NOTIFICATION &err = n->info.err;
            ret = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
                "k(y#iiN)",
#else
                "k(s#iiN)",
#endif
                n->ulEventType, err.lpEntryID, err.cbEntryID, err.scode, err.ulFlags,
                PyObject_FromMAPIERROR(err.lpMAPIError, err.ulFlags & MAPI_UNICODE, FALSE));
            break;
        }
        case fnevExtended: {
            EXTENDED_NOTIFICATION &ext = n->info.ext;
            ret = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
                "k(ky#)",
#else
                "k(ks#)",
#endif
                n->ulEventType, ext.ulEvent, ext.pbEventParameters, ext.cb);
            break;
        }
        case fnevNewMail: {
            NEWMAIL_NOTIFICATION &newmail = n->info.newmail;
            PyObject *msg_class = newmail.ulFlags & MAPI_UNICODE
                                      ? PyWinObject_FromWCHAR((const WCHAR *)newmail.lpszMessageClass)
                                      : PyBytes_FromString((const char *)newmail.lpszMessageClass);
            if (!msg_class)
                return NULL;
            ret = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
                "k(y#y#kNk)",
#else
                "k(s#s#kNk)",
#endif
                n->ulEventType, newmail.lpEntryID, newmail.cbEntryID, newmail.lpParentID, newmail.cbParentID,
                newmail.ulFlags, msg_class, newmail.ulMessageFlags);
            break;
        }
        case fnevObjectCopied:
        case fnevObjectCreated:
        case fnevObjectDeleted:
        case fnevObjectModified:
        case fnevObjectMoved:
        case fnevSearchComplete: {
            OBJECT_NOTIFICATION &obj = n->info.obj;
            PyObject *obArray = PyMAPIObject_FromSPropTagArray(obj.lpPropTagArray);
            if (!obArray)
                return NULL;
            ret = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
                "k(y#iy#y#y#N)",
#else
                "k(s#is#s#s#N)",
#endif
                n->ulEventType, obj.lpEntryID, obj.cbEntryID, obj.ulObjType, obj.lpParentID, obj.cbParentID,
                obj.lpOldID, obj.cbOldID, obj.lpOldParentID, obj.cbOldParentID, obArray);
            break;
        }
        case fnevTableModified: {
            TABLE_NOTIFICATION &tab = n->info.tab;
            ret = Py_BuildValue("k(kiNNN)", n->ulEventType, tab.ulTableEvent, tab.hResult,
                                PyMAPIObject_FromSPropValue(&tab.propIndex),
                                PyMAPIObject_FromSPropValue(&tab.propPrior), PyMAPIObject_FromSRow(&tab.row));
            break;
        }
        case fnevStatusObjectModified: {
            STATUS_OBJECT_NOTIFICATION &statobj = n->info.statobj;
            ret = Py_BuildValue(
#if PY_MAJOR_VERSION >= 3
                "k(y#N)",
#else
                "k(s#N)",
#endif
                n->ulEventType, statobj.lpEntryID, statobj.cbEntryID,
                PyMAPIObject_FromSPropValueArray(statobj.lpPropVals, statobj.cValues));
            break;
        }
        default: {
            PyCom_LoggerWarning(NULL, L"unknown MAPI notification type %x", n->ulEventType);
            ret = Py_BuildValue("k(O)", n->ulEventType, Py_None);
            break;
        }
    }
    return ret;
}

ULONG PyGMAPIAdviseSink::OnNotify(ULONG cNotif, LPNOTIFICATION lpNotifications)
{
    PY_GATEWAY_METHOD;
    PyObject *arg = PyList_New(cNotif);
    if (!arg)
        return MAKE_PYCOM_GATEWAY_FAILURE_CODE("OnNotify");
    ULONG i;
    for (i = 0; i < cNotif; i++) {
        PyObject *sub = PyObject_FromNOTIFICATION(lpNotifications + i);
        if (!sub) {
            Py_DECREF(arg);
            return MAKE_PYCOM_GATEWAY_FAILURE_CODE("OnNotify");
        }
        PyList_SET_ITEM(arg, i, sub);
    }
    return InvokeViaPolicy("OnNotify", NULL, "(N)", arg);
}

PyIMAPIAdviseSink::PyIMAPIAdviseSink(IUnknown *pDisp) : PyIUnknown(pDisp) { ob_type = &type; }

PyIMAPIAdviseSink::~PyIMAPIAdviseSink() {}

/*static*/ IMAPIAdviseSink *PyIMAPIAdviseSink::GetI(PyObject *self)
{
    return (IMAPIAdviseSink *)PyIUnknown::GetI(self);
}

static PyMethodDef IMAPIAdviseSinkMethods[] = {{NULL, NULL}};

PyComTypeObject PyIMAPIAdviseSink::type("PyIMAPIAdviseSink", &PyIUnknown::type, sizeof(PyIMAPIAdviseSink),
                                        IMAPIAdviseSinkMethods, GET_PYCOM_CTOR(PyIMAPIAdviseSink));

PyCMAPIAdviseSink::PyCMAPIAdviseSink(PyObject *callback, PyObject *context)
{
    m_cRef = 1;
    Py_INCREF(callback);
    m_callback = callback;
    Py_INCREF(context);
    m_context = context;
}

PyCMAPIAdviseSink::~PyCMAPIAdviseSink()
{
    Py_DECREF(m_callback);
    Py_DECREF(m_context);
}

STDMETHODIMP PyCMAPIAdviseSink::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    *ppvObj = 0;
    if (riid == IID_IMAPIAdviseSink || riid == IID_IUnknown) {
        *ppvObj = (LPVOID)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) PyCMAPIAdviseSink::AddRef()
{
    LONG lCount = InterlockedIncrement(&m_cRef);
    return lCount;
}

STDMETHODIMP_(ULONG) PyCMAPIAdviseSink::Release()
{
    LONG lCount = InterlockedDecrement(&m_cRef);
    if (!lCount)
        delete this;
    return lCount;
}

ULONG PyCMAPIAdviseSink::OnNotify(ULONG cNotif, LPNOTIFICATION lpNotifications)
{
    PY_GATEWAY_METHOD;

    PyObject *arg = PyTuple_New(cNotif);
    if (!arg) {
        PyCom_LoggerException(NULL, L"File %s:Line %d:%s:PyTuple_New() failed.", __FILE__, __LINE__, __FUNCSIG__);
        PyErr_Clear();
        return 0;
    }
    for (ULONG i = 0; i < cNotif; i++) {
        PyObject *sub = PyObject_FromNOTIFICATION(lpNotifications + i);
        if (!sub) {
            Py_DECREF(arg);
            PyCom_LoggerException(NULL, L"File %s:Line %d:%s:PyObject_FromNOTIFICATION failed.", __FILE__, __LINE__,
                                  __FUNCSIG__);
            PyErr_Clear();
            return 0;
        }
        PyTuple_SET_ITEM(arg, i, sub);
    }
    PyObject *args = Py_BuildValue("NO", arg, m_context);
    if (!args) {
        PyCom_LoggerException(NULL, L"File %s:Line %d:%s:Py_BuildValue() failed.", __FILE__, __LINE__, __FUNCSIG__);
        PyErr_Clear();
        return 0;
    }

    PyObject *result = PyObject_CallObject(m_callback, args);
    Py_DECREF(args);
    if (!result) {
        PyCom_LoggerException(NULL, L"File %s:Line %d:%s:PyObject_CallObject failed.", __FILE__, __LINE__, __FUNCSIG__);
        PyErr_Clear();
        return 0;
    }
    Py_DECREF(result);
    return 0;
}
