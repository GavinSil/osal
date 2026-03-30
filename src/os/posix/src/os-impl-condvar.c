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
#include "os-shared-condvar.h"
#include "os-shared-idmap.h"
#include "os-impl-condvar.h"
#include "os-posix-stepping.h"

#ifdef CFE_SIM_STEPPING
#include "esa_stepping.h"
#include "esa_wait.h"
extern bool ESA_Stepping_Hook_IsSessionActive(void);
#endif

/* Tables where the OS object information is stored */
OS_impl_condvar_internal_record_t OS_impl_condvar_table[OS_MAX_CONDVARS];

/*---------------------------------------------------------------------------------------
 * Helper function for releasing the mutex in case the thread
 * executing pthread_cond_wait() is canceled.
 ----------------------------------------------------------------------------------------*/
static void OS_Posix_CondVarReleaseMutex(void *mut)
{
    pthread_mutex_unlock(mut);
}

/****************************************************************************************
                                  CONDVAR API
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_Posix_CondVarAPI_Impl_Init(void)
{
    memset(OS_impl_condvar_table, 0, sizeof(OS_impl_condvar_table));
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarCreate_Impl(const OS_object_token_t *token, uint32 options)
{
    int32                              final_status;
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    final_status = OS_SUCCESS;
    impl         = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    /*
    ** create the underlying mutex
    */
    status = pthread_mutex_init(&impl->mut, NULL);
    if (status != 0)
    {
        OS_DEBUG("Error: CondVar mutex could not be created. ID = %lu: %s\n",
                 OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)), strerror(status));
        final_status = OS_ERROR;
    }
    else
    {
        /*
        ** create the condvar
        */
        status = pthread_cond_init(&impl->cv, NULL);
        if (status != 0)
        {
            pthread_mutex_destroy(&impl->mut);

            OS_DEBUG("Error: CondVar could not be created. ID = %lu: %s\n",
                     OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)), strerror(status));
            final_status = OS_ERROR;
        }
    }

    return final_status;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarDelete_Impl(const OS_object_token_t *token)
{
    int32                              final_status;
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    final_status = OS_SUCCESS;
    impl         = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_destroy(&impl->cv);
    if (status != 0)
    {
        final_status = OS_ERROR;
    }

    status = pthread_mutex_destroy(&impl->mut);
    if (status != 0)
    {
        final_status = OS_ERROR;
    }

    return final_status;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarUnlock_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_mutex_unlock(&impl->mut);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarLock_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_mutex_lock(&impl->mut);
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarSignal_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_signal(&impl->cv);
    if (status != 0)
    {
        return OS_ERROR;
    }

#ifdef CFE_SIM_STEPPING
    /* Notify stepping system on successful signal */
    ESA_NotifyCondVar(OS_ObjectIdFromToken(token), false);
#endif

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarBroadcast_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

    status = pthread_cond_broadcast(&impl->cv);
    if (status != 0)
    {
        return OS_ERROR;
    }

#ifdef CFE_SIM_STEPPING
    /* Notify stepping system on successful broadcast */
    ESA_NotifyCondVar(OS_ObjectIdFromToken(token), true);
#endif

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarWait_Impl(const OS_object_token_t *token)
{
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

#ifdef CFE_SIM_STEPPING
    bool use_esa_wait;

    /*
     * Only use ESA wait path if stepping session is active
     */
    use_esa_wait = ESA_Stepping_Hook_IsSessionActive();

    if (use_esa_wait)
    {
        int32     return_code;
        osal_id_t condvar_id;
        int32     esa_result;

        return_code = OS_SUCCESS;

        /*
         * ESA stepping mode: use ESA_WaitForCondVar for deterministic waits
         */
        while (true)
        {
            condvar_id = OS_ObjectIdFromToken(token);

            /*
             * Release the mutex before calling ESA_WaitForCondVar to avoid deadlock
             */
            pthread_mutex_unlock(&impl->mut);

            /*
             * Use ESA_WaitForCondVar with OS_PEND (infinite wait)
             */
            esa_result = ESA_WaitForCondVar(condvar_id, OS_PEND);

            /*
             * Re-acquire the mutex after waiting
             */
            if (pthread_mutex_lock(&impl->mut) != 0)
            {
                return_code = OS_ERROR;
                break;
            }

            if (esa_result == ESA_WOKE_BY_RESOURCE)
            {
                /* Woken by signal/broadcast - return success */
                break;
            }
            else if (esa_result < 0)
            {
                return_code = OS_ERROR;
                break;
            }
            /* else: continue waiting */
        }

        return return_code;
    }
    /* Fall through to legacy path if !use_esa_wait */
#endif

    /*
     * Legacy POSIX condvar implementation
     * note that because pthread_cond_wait is a cancellation point, this needs to
     * employ the same protection that is in the binsem module.  In the event that
     * the thread is canceled inside pthread_cond_wait, the mutex will be re-acquired
     * before the cancellation occurs, leaving the mutex in a locked state.
     */
    pthread_cleanup_push(OS_Posix_CondVarReleaseMutex, &impl->mut);
    status = pthread_cond_wait(&impl->cv, &impl->mut);
    pthread_cleanup_pop(false);

    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarTimedWait_Impl(const OS_object_token_t *token, const OS_time_t *abs_wakeup_time)
{
    struct timespec                    limit;
    int                                status;
    OS_impl_condvar_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_condvar_table, *token);

#ifdef CFE_SIM_STEPPING
    bool use_esa_wait;

    /*
     * Only use ESA wait path if stepping session is active
     */
    use_esa_wait = ESA_Stepping_Hook_IsSessionActive();

    if (use_esa_wait)
    {
        int32     return_code;
        osal_id_t condvar_id;
        int32     esa_result;
        uint64    current_sim_time_ns;
        int64     remaining_ns;
        uint32    timeout_ms;

        return_code = OS_SUCCESS;

        while (true)
        {
            condvar_id = OS_ObjectIdFromToken(token);

            /*
             * Calculate remaining timeout using simulated time
             */
            if (!ESA_Stepping_Hook_GetTime(&current_sim_time_ns))
            {
                return_code = OS_ERROR;
                break;
            }

            remaining_ns = (int64)(OS_TimeGetTotalSeconds(*abs_wakeup_time) * 1000000000LL +
                                   OS_TimeGetNanosecondsPart(*abs_wakeup_time)) -
                           (int64)current_sim_time_ns;

            if (remaining_ns <= 0)
            {
                /* Already past deadline */
                return_code = OS_ERROR_TIMEOUT;
                break;
            }

            /*
             * Convert nanoseconds to milliseconds (round up to avoid premature timeout)
             */
            timeout_ms = (uint32)((remaining_ns + 999999) / 1000000);

            /*
             * Release the mutex before calling ESA_WaitForCondVar to avoid deadlock
             */
            pthread_mutex_unlock(&impl->mut);

            /*
             * Use ESA_WaitForCondVar with calculated timeout
             */
            esa_result = ESA_WaitForCondVar(condvar_id, timeout_ms);

            /*
             * Re-acquire the mutex after waiting
             */
            if (pthread_mutex_lock(&impl->mut) != 0)
            {
                return_code = OS_ERROR;
                break;
            }

            if (esa_result == ESA_WOKE_BY_TIMEOUT)
            {
                return_code = OS_ERROR_TIMEOUT;
                break;
            }
            else if (esa_result == ESA_WOKE_BY_RESOURCE)
            {
                /* Woken by signal/broadcast - return success */
                break;
            }
            else if (esa_result < 0)
            {
                return_code = OS_ERROR;
                break;
            }
            /* else: continue waiting */
        }

        return return_code;
    }
    /* Fall through to legacy path if !use_esa_wait */
#endif

    /*
     * Legacy POSIX timedwait implementation
     */
    limit.tv_sec  = OS_TimeGetTotalSeconds(*abs_wakeup_time);
    limit.tv_nsec = OS_TimeGetNanosecondsPart(*abs_wakeup_time);

    pthread_cleanup_push(OS_Posix_CondVarReleaseMutex, &impl->mut);
    status = pthread_cond_timedwait(&impl->cv, &impl->mut, &limit);
    pthread_cleanup_pop(false);

    if (status == ETIMEDOUT)
    {
        return OS_ERROR_TIMEOUT;
    }
    if (status != 0)
    {
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_CondVarGetInfo_Impl(const OS_object_token_t *token, OS_condvar_prop_t *condvar_prop)
{
    return OS_SUCCESS;
}
