#ifndef ElementOS_APP_System_event
#define ElementOS_APP_System_event

#include "../elementos.hpp"
#include "../elementos-std.hpp"
#include "../conf/elementos-namecase.hpp"
#include "../inc/lfarray.hpp"
#include <Arduino.h>

elestd_package System{
    elestd_application event : public elestd_baseApplication{
        public:
            event() : elestd_baseApplication(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event){}

            elestd_export start(void* obj){
                event *thisobj = (event*)obj;
                ElementOS::Api->addRemoteCallToApplication<event>(thisobj,ElementOS::n_createEvent,&(event::createEvent));
                ElementOS::Api->addRemoteCallToApplication<event>(thisobj,ElementOS::n_removeEvent,&(event::removeEvent));
                ElementOS::Api->addRemoteCallToApplication<event>(thisobj,ElementOS::n_addEventReceiver,&(event::addEventReceiver));
                ElementOS::Api->addRemoteCallToApplication<event>(thisobj,ElementOS::n_removeEventReceiver,&(event::removeEventReceiver));
                ElementOS::Api->addRemoteCallToApplication<event>(thisobj,ElementOS::n_raiseEvent,&(event::raiseEvent));
            }

            elestd_export createEvent(TaskHandle_t* task,void* obj,ElementOSAPI::va_list_Sender *args){
                event *thisobj = (event*)obj;
                elestd_name eventName = (elestd_name)(args->getArg<int>());
                args->end();
                if (thisobj->eventName.includes(eventName)){
                    return;
                }
                thisobj->eventName.push(eventName);
            }

            elestd_export removeEvent(TaskHandle_t* task,void* obj,ElementOSAPI::va_list_Sender *args){
                event *thisobj = (event*)obj;
                elestd_name eventName = (elestd_name)(args->getArg<int>());
                args->end();
                if (!(thisobj->eventName.includes(eventName))){
                    return;
                }
                thisobj->eventName.remove(thisobj->eventName.indexOf(eventName));
            }

            elestd_export addEventReceiver(TaskHandle_t* task,void* obj,ElementOSAPI::va_list_Sender *args){
                event *thisobj = (event*)obj;
                ElementOSAPI::applicationRemoteCallInfo *eventReceiver = (ElementOSAPI::applicationRemoteCallInfo*)(args->getArg<void*>());
                args->end();
                if (thisobj->eventReceiver.includes(*eventReceiver)){
                    return;
                }
                thisobj->eventReceiver.push(*eventReceiver);
            }

            elestd_export removeEventReceiver(TaskHandle_t* task,void* obj,ElementOSAPI::va_list_Sender *args){
                event *thisobj = (event*)obj;
                ElementOSAPI::applicationRemoteCallInfo *eventReceiver = (ElementOSAPI::applicationRemoteCallInfo*)(args->getArg<void*>());
                args->end();
                if (!(thisobj->eventReceiver.includes(*eventReceiver))){
                    return;
                }
                thisobj->eventReceiver.remove(thisobj->eventReceiver.indexOf(*eventReceiver));
            }

            elestd_export raiseEvent(TaskHandle_t* task,void* obj,ElementOSAPI::va_list_Sender *args){
                event *thisobj = (event*)obj;
                elestd_name eventName = (elestd_name)(args->getArg<int>());
                args->end();
                if (!(thisobj->eventName.includes(eventName))){
                    return;
                }
                for (long i=0;i<thisobj->eventReceiver.length();i++){
                    ElementOS::Api->callApplicationRemoteCall(thisobj->eventReceiver[i],thisobj->eventReceiverStackDeep[i],eventName);
                }
                
            }

        private:
            lfArray<elestd_name> eventName;
            lfArray<ElementOSAPI::applicationRemoteCallInfo> eventReceiver;
            lfArray<configSTACK_DEPTH_TYPE> eventReceiverStackDeep;

    };
}

elestd_addin{
    namespace eventApi{
        void createEvent(elestd_name eventName){
            ElementOS::Api->callApplicationRemoteCall(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event,1024,ElementOS::n_createEvent,eventName);
        }

        void removeEvent(elestd_name eventName){
            ElementOS::Api->callApplicationRemoteCall(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event,1024,ElementOS::n_removeEvent,eventName);
        }

        void addEventReceiver(ElementOSAPI::applicationRemoteCallInfo *receiverInfo){
            ElementOS::Api->callApplicationRemoteCall(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event,1024,ElementOS::n_addEventReceiver,receiverInfo);
        }

        void removeEventReceiver(ElementOSAPI::applicationRemoteCallInfo *receiverInfo){
            ElementOS::Api->callApplicationRemoteCall(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event,1024,ElementOS::n_removeEventReceiver,receiverInfo);
        }

        void raiseEvent(elestd_name eventName){
            ElementOS::Api->callApplicationRemoteCall(ElementOS::n_System,ElementOS::n_event,ElementOS::n_System_event,1024,ElementOS::n_raiseEvent,eventName);
        }
    }
}


#endif