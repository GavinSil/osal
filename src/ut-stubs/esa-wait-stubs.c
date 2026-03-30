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

#include "common_types.h"
#include "osapi-idmap.h"
#include "utstub-helpers.h"

int32 ESA_WaitForMessage(osal_id_t queue_id, uint32 timeout_ms)
{
    int32 status;

    status = UT_DEFAULT_IMPL(ESA_WaitForMessage);

    return status;
}

int32 ESA_WaitForSem(osal_id_t sem_id, uint32 timeout_ms)
{
    int32 status;

    status = UT_DEFAULT_IMPL(ESA_WaitForSem);

    return status;
}

int32 ESA_WaitForCondVar(osal_id_t condvar_id, uint32 timeout_ms)
{
    int32 status;

    status = UT_DEFAULT_IMPL(ESA_WaitForCondVar);

    return status;
}

int32 ESA_WaitForDelay(uint32 timeout_ms)
{
    int32 status;

    status = UT_DEFAULT_IMPL(ESA_WaitForDelay);

    return status;
}

void ESA_NotifyQueuePut(osal_id_t queue_id)
{
    UT_DEFAULT_IMPL(ESA_NotifyQueuePut);
}

void ESA_NotifySemGive(osal_id_t sem_id)
{
    UT_DEFAULT_IMPL(ESA_NotifySemGive);
}

void ESA_NotifySemFlush(osal_id_t sem_id)
{
    UT_DEFAULT_IMPL(ESA_NotifySemFlush);
}

void ESA_NotifyCondVar(osal_id_t condvar_id, bool broadcast)
{
    UT_DEFAULT_IMPL(ESA_NotifyCondVar);
}

void ESA_RegisterTask(osal_id_t task_id)
{
    UT_DEFAULT_IMPL(ESA_RegisterTask);
}

void ESA_DeregisterTask(osal_id_t task_id)
{
    UT_DEFAULT_IMPL(ESA_DeregisterTask);
}
