#ifndef ElementOS_std
#define ElementOS_std

#include "conf/elementos-namecase.hpp"

// keywords

#define elestd_package namespace
#define elestd_application class
#define elestd_export static void
typedef ElementOS::nameCase elestd_name;
#define elestd_addin namespace ElementOS

// structs

elestd_application elestd_baseApplication{
    public:
        const elestd_name packageName;
        const elestd_name appClassName;
        const elestd_name mixName;

        elestd_baseApplication(elestd_name mPackageName,elestd_name mAppClassName,elestd_name mMixName):
        packageName(mPackageName),
        appClassName(mAppClassName),
        mixName(mMixName)
        {}

        elestd_export start(void* obj){}
        elestd_export end(void* obj){}
        elestd_export suspend(void* obj){}
        elestd_export resume(void* obj){}

};

#endif