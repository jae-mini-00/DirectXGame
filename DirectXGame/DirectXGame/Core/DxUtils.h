#pragma once

#include <windows.h>

#define RETURN_IF_FAILED(expr) \
    do { \
        HRESULT result__ = (expr); \
        if (FAILED(result__)) \
            return result__; \
    } while (0)

#define RETURN_IF_FAILED_AND_RELEASE(expr, resource) \
    do { \
        HRESULT hr = (expr); \
        if (FAILED(hr)) {\
            if (resource) {\
                resource->Release();\
                resource = nullptr;\
            }\
            return hr; \
            }\
    } while (0)