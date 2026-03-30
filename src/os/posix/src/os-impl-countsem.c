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
 * \author   joseph.p.hickey@nasa.gov
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-posix.h"
#include "os-impl-countsem.h"
#include "os-shared-countsem.h"
#include "os-shared-idmap.h"
#include "os-posix-stepping.h"

#ifdef CFE_SIM_STEPPING
#include "esa_stepping.h"
#include "esa_wait.h"
extern bool ESA_Stepping_Hook_IsSessionActive(void);
#endif

/*
 * Added SEM_VALUE_MAX Define
 */
#ifndef SEM_VALUE_MAX
#define SEM_VALUE_MAX (UINT32_MAX / 2)
#endif

/* Tables where the OS object information is stored */
OS_impl_countsem_internal_record_t OS_impl_count_sem_table[OS_MAX_COUNT_SEMAPHORES];

/****************************************************************************************
                               COUNTING SEMAPHORE API
 ***************************************************************************************/

/*
 * Unlike binary semaphores, counting semaphores can use the standard POSIX semaphore facility.
 * This has the advantage of more correct behavior on "give" operations:
 *  - give may be done from a signal / ISR context
 *  - give should not cause an unexpected task switch nor should it ever block
 */

/*---------------------------------------------------------------------------------------
   Name: OS_Posix_CountSemAPI_Impl_Init

   Purpose: Initialize the Counting Semaphore data structures

---------------------------------------------------------------------------------------*/
int32 OS_Posix_CountSemAPI_Impl_Init(void)
{
    memset(OS_impl_count_sem_table, 0, sizeof(OS_impl_count_sem_table));
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemCreate_Impl(const OS_object_token_t *token, uint32 sem_initial_value, uint32 options)
{
    OS_impl_countsem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if (sem_initial_value > SEM_VALUE_MAX)
    {
        return OS_INVALID_SEM_VALUE;
    }

    if (sem_init(&impl->id, 0, sem_initial_value) < 0)
    {
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if (sem_destroy(&impl->id) < 0)
    {
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGive_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if (sem_post(&impl->id) < 0)
    {
        return OS_SEM_FAILURE;
    }

#ifdef CFE_SIM_STEPPING
    ESA_NotifySemGive(OS_ObjectIdFromToken(token));
#endif

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTake_Impl(const OS_object_token_t *token)
{
    OS_impl_countsem_internal_record_t *impl;
#ifdef CFE_SIM_STEPPING
    int32     esa_result;
    osal_id_t sem_id;
    bool      use_esa_wait;
#endif

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

#ifdef CFE_SIM_STEPPING
    /* Check if stepping session is active before using ESA wait */
    use_esa_wait = ESA_Stepping_Hook_IsSessionActive();
    
    if (use_esa_wait)
    {
        /* Stepping mode: Non-blocking try → ESA wait → retry loop */
        while (true)
        {
            /* Attempt non-blocking acquire */
            if (sem_trywait(&impl->id) == 0)
            {
                return OS_SUCCESS;
            }

            /* Handle errno from sem_trywait */
            if (errno == EINTR)
            {
                /* Interrupted by signal, retry immediately */
                continue;
            }
            else if (errno == EAGAIN)
            {
                /* Resource not available, wait via ESA */
                sem_id     = OS_ObjectIdFromToken(token);
                esa_result = ESA_WaitForSem(sem_id, OS_PEND);

                if (esa_result == ESA_WOKE_BY_RESOURCE)
                {
                    /* Woken by resource, retry acquire (another thread may have taken it) */
                    continue;
                }
                else if (esa_result < 0)
                {
                    /* ESA wait error */
                    return OS_SEM_FAILURE;
                }
                /* ESA_WOKE_BY_FLUSH also continues loop to retry acquire */
            }
            else
            {
                /* Other error (EINVAL, etc.) */
                return OS_SEM_FAILURE;
            }
        }
    }
    /* Fall through to legacy POSIX path if session not active */
#endif
    
    /* Non-stepping mode or stepping without active session: Traditional blocking wait */
    if (sem_wait(&impl->id) < 0)
    {
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemTimedWait_Impl(const OS_object_token_t *token, uint32 msecs)
{
#ifdef CFE_SIM_STEPPING
    OS_impl_countsem_internal_record_t *impl;
    int32                                esa_result;
    osal_id_t                            sem_id;
    bool                                 use_esa_wait;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    /* Check if stepping session is active before using ESA wait */
    use_esa_wait = ESA_Stepping_Hook_IsSessionActive();

    if (use_esa_wait)
    {
        /* Stepping mode: Non-blocking try → ESA timed wait → retry loop */
        while (true)
        {
            /* Attempt non-blocking acquire */
            if (sem_trywait(&impl->id) == 0)
            {
                return OS_SUCCESS;
            }

            /* Handle errno from sem_trywait */
            if (errno == EINTR)
            {
                /* Interrupted by signal, retry immediately */
                continue;
            }
            else if (errno == EAGAIN)
            {
                /* Resource not available, wait via ESA with timeout */
                sem_id     = OS_ObjectIdFromToken(token);
                esa_result = ESA_WaitForSem(sem_id, msecs);

                if (esa_result == ESA_WOKE_BY_TIMEOUT)
                {
                    /* Timeout expired */
                    return OS_SEM_TIMEOUT;
                }
                else if (esa_result == ESA_WOKE_BY_RESOURCE)
                {
                    /* Woken by resource, retry acquire (another thread may have taken it) */
                    continue;
                }
                else if (esa_result < 0)
                {
                    /* ESA wait error */
                    return OS_SEM_FAILURE;
                }
                /* ESA_WOKE_BY_FLUSH also continues loop to retry acquire */
            }
            else
            {
                /* Other error (EINVAL, etc.) */
                return OS_SEM_FAILURE;
            }
        }
    }
    /* Fall through to legacy POSIX path if session not active */
#endif
    
    /* Non-stepping mode or stepping without active session: Traditional sem_timedwait with absolute time */
    {
        struct timespec                     ts;
        int                                 result;
        OS_impl_countsem_internal_record_t *impl;

        impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

        /*
         ** Compute an absolute time for the delay
         */
        OS_Posix_CompAbsDelayTime(msecs, &ts);

        if (sem_timedwait(&impl->id, &ts) == 0)
        {
            result = OS_SUCCESS;
        }
        else if (errno == ETIMEDOUT)
        {
            result = OS_SEM_TIMEOUT;
        }
        else
        {
            /* unspecified failure */
            result = OS_SEM_FAILURE;
        }

        return result;
    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CountSemGetInfo_Impl(const OS_object_token_t *token, OS_count_sem_prop_t *count_prop)
{
    int                                 sval;
    OS_impl_countsem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if (sem_getvalue(&impl->id, &sval) < 0)
    {
        return OS_SEM_FAILURE;
    }

    /* put the info into the structure */
    count_prop->value = sval;
    return OS_SUCCESS;
}
