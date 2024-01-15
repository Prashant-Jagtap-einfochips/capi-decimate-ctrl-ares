# Sensor Utility

##  Introduction

Certain modules have a need to communicate with sensors in order to get the relevant data on 
which they can act. One such example that is also available on SDK is UPD module where the 
detector module is required to access info from sensors. In order to get info from sensors, 
there are certain APIs that are required to be used. This document serves the purpose of 
describing sensor utilities and how can they be used.

## APIs

sns_utils_init(void **sns_utils_inst_pptr, uint32_t req_miid, char const *  sensor_str_ptr, POSAL_HEAP_ID heap_id): 

 * The below function gets invoked in the event requester module's initialization sequence.
 * It allocates the sns util instance and the event payload memory. It also initializes a  
   Qsocket client and sends a request to the SUID service to look up the requested sensor in the db.
 * sns_utils_inst_pptr  -> double pointer to sns_utils instance handle (client_handle_pptr)
 * sensor_str_ptr -> name of the sensor (String)
 * req_miid -> module instance ID of the requester (CAPI module)
 * heap_id -> Heap id that needs to be used for memory allocation.
 

sns_utils_register(void *client_handle_ptr):

 * The below function gets invoked in the event requester module's initialization sequence after sns_utils_init.
 * This function checks if SUID service is up, and acked the suid request/indicated the suid.
 * It also places asynchronous message to request for indications from Requested sensor Service for ON_CHANGE events
 * client_handle_ptr -> client handle


sns_utils_poll_and_get_requested_events(void *client_handle_ptr, void** event_payload_ptr):

 * The below function gets invoked whenever registered events are required by the client (ex: CAPI)
 * It will return the event payload if available, if not, it returns NULL, and an error code 
 * AR_EOK -> Event indication done and payload pointer is valid
 * AR_ENOTREADY -> Either Event registration is not done yet, or Event indication is not received yet. 
                   Payload pointer will be NULL, client is free to use its own default
				   
				   
sns_utils_destroy(void **client_handle_pptr):

 * The below function gets invoked when destroying the service to free memory allocated for :
 * the sns util instance and the event payload memory
 * client_handle_pptr  -> double pointer to sns_utils instance handle
 
 
sns_utils_deregister(void *client_handle_ptr):

 * The below function gets invoked in  stop cmnd to deregister from  sns_utils
 * This function will send a disable request to the sensor
 * It will also close the qsockets connections 
 * client_handle_ptr -> client handle
 