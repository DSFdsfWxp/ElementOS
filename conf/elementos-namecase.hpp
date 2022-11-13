#ifndef ElementOS_nameCase
#define ElementOS_nameCase

/*
*   TO DO: 
*   Define the elestd_name you want to use in
*   your project.
*/

namespace ElementOS{
    enum nameCase{
        n_NULL,
        n_noneNameTask,
        n_start,
        n_suspend,
        n_resume,
        n_end,
        n_appRemoteCall,
        n_appRemoveCall,
        n_foregroundApp,
        n_foregroundApp_packageName,
        n_foregroundApp_appClassName,
        n_foregroundApp_mixName,
        n_foregroundApp_endStackDeep,
        n_dev,
        n_System,
        n_core,
        n_System_core,
        n_event,
        n_System_event,
        n_createEvent,
        n_removeEvent,
        n_raiseEvent,
        n_addEventReceiver,
        n_removeEventReceiver,
        n_LED0,
        n_task1
    };
}

#endif