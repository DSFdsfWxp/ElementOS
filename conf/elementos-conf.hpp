#ifndef ElementOS_conf
#define ElementOS_conf

#define ElementOS_conf_enableDebug 0

#define ElementOS_conf_enableTaskTracking 0


/*
*   fix Parts
*/

// if configSTACK_DEPTH_TYPE is not defined, enable it.
#define ElementOS_conf_fixStackDeepType 0

// if ledcChangeFrequency(...) is not defined, enable it.
#define ElementOS_conf_fixLedPwm 0


#endif