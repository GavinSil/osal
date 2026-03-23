/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 * \ingroup  posix
 *
 * Purpose: This file contains POSIX-specific stepping/simulation hook implementations
 *          that are called at synchronization boundaries when CFE_SIM_STEPPING is enabled.
 *
 * These are stub implementations that can be extended to implement deterministic
 * time stepping functionality.
 */

#ifdef CFE_SIM_STEPPING

#include <stdint.h>
#include "os-posix-stepping.h"
#include "os-shared-idmap.h"
#include "osapi-idmap.h"
#include "osapi-task.h"
#include "esa_stepping_shim.h"
#include "esa_stepping.h"

extern int32_t ESA_Stepping_Shim_ReportEvent(const ESA_Stepping_ShimEvent_t *event) __attribute__((weak));

/****************************************************************************************
                                  STEPPING HOOK IMPLEMENTATIONS
 ***************************************************************************************/

/**
 * \brief Hook called at task delay boundary
 *
 * Stub implementation that is called before and after a task delay operation.
 * Real implementations can use this to synchronize deterministic time progression.
 *
 * \param[in]   ms          Number of milliseconds the task is delaying
 */
void OS_PosixStepping_Hook_TaskDelay(uint32_t ms, osal_id_t task_id)
{
    ESA_Stepping_ShimEvent_t event = {0};

    event.event_kind = ESA_SIM_STEPPING_EVENT_TASK_DELAY_ACK;
    event.task_id = (uint32_t)task_id;
    event.optional_delay_ms = ms;

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }
}

void OS_PosixStepping_Hook_TaskDelay_Complete(uint32_t ms, osal_id_t task_id)
{
    ESA_Stepping_ShimEvent_t event = {0};

    event.event_kind = ESA_SIM_STEPPING_EVENT_TASK_DELAY_COMPLETE;
    event.task_id = (uint32_t)task_id;
    event.optional_delay_ms = ms;

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }
}

/**
 * \brief Hook called at queue receive boundary
 *
 * Stub implementation that is called when a task attempts to receive a message from a queue.
 * Real implementations can use this to manage message delivery timing in deterministic simulations.
 */
void OS_PosixStepping_Hook_QueueReceive(const OS_object_token_t *token, int32 timeout)
{
    ESA_Stepping_ShimEvent_t event    = {0};
    osal_id_t                       task_id  = OS_TaskGetId();
    osal_id_t                       queue_id = OS_ObjectIdFromToken(token);

    event.event_kind         = ESA_SIM_STEPPING_EVENT_QUEUE_RECEIVE_ACK;
    event.entity_id          = (uint32_t)OS_ObjectIdToInteger(queue_id);
    event.task_id            = (uint32_t)OS_ObjectIdToInteger(task_id);
    event.optional_delay_ms  = (uint32_t)timeout;

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }
}

void OS_PosixStepping_Hook_QueueReceive_Complete(const OS_object_token_t *token, int32 timeout, int32 return_code)
{
    ESA_Stepping_ShimEvent_t event    = {0};
    osal_id_t                       task_id  = OS_TaskGetId();
    osal_id_t                       queue_id = OS_ObjectIdFromToken(token);

    event.event_kind         = ESA_SIM_STEPPING_EVENT_QUEUE_RECEIVE_COMPLETE;
    event.entity_id          = (uint32_t)OS_ObjectIdToInteger(queue_id);
    event.task_id            = (uint32_t)OS_ObjectIdToInteger(task_id);
    event.optional_delay_ms  = (uint32_t)timeout;

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }

    (void)return_code;
}

/**
 * \brief Hook called at binary semaphore take boundary
 *
 * Stub implementation that is called when a task attempts to take a binary semaphore.
 * Real implementations can use this to manage synchronization points in deterministic simulations.
 */
void OS_PosixStepping_Hook_BinSemTake(const OS_object_token_t *token, const struct timespec *timeout)
{
    ESA_Stepping_ShimEvent_t event = {0};

    event.event_kind = ESA_SIM_STEPPING_EVENT_BINSEM_TAKE_ACK;
    event.entity_id  = (uint32_t)OS_ObjectIdToInteger(OS_ObjectIdFromToken(token));
    event.task_id    = (uint32_t)OS_ObjectIdToInteger(OS_TaskGetId());

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }

    (void)timeout;
}

void OS_PosixStepping_Hook_BinSemTake_Complete(const OS_object_token_t *token, const struct timespec *timeout,
                                               int32 return_code)
{
    ESA_Stepping_ShimEvent_t event = {0};

    event.event_kind = ESA_SIM_STEPPING_EVENT_BINSEM_TAKE_COMPLETE;
    event.entity_id  = (uint32_t)OS_ObjectIdToInteger(OS_ObjectIdFromToken(token));
    event.task_id    = (uint32_t)OS_ObjectIdToInteger(OS_TaskGetId());

    if (ESA_Stepping_Shim_ReportEvent != NULL)
    {
        ESA_Stepping_Shim_ReportEvent(&event);
    }

    (void)timeout;
    (void)return_code;
}

#endif /* CFE_SIM_STEPPING */
