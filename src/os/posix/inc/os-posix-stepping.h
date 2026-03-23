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
 * Purpose: This file contains POSIX-specific stepping/simulation hooks
 *          that are called at synchronization boundaries when CFE_SIM_STEPPING is enabled.
 *
 * These hooks allow simulation stepping implementations to intercept task delays,
 * queue receives, and binary semaphore waits to implement deterministic time stepping.
 */

#ifndef OS_POSIX_STEPPING_H
#define OS_POSIX_STEPPING_H

/****************************************************************************************
                                     INCLUDE FILES
 ***************************************************************************************/

#include <stdint.h>
#include "os-posix.h"

/****************************************************************************************
                            STEPPING HOOK DECLARATIONS
 ***************************************************************************************/

/**
 * \brief Hook called at task delay boundary
 *
 * Called before and after a task delay operation to allow stepping implementations
 * to synchronize deterministic time progression.
 *
 * \param[in]   ms          Number of milliseconds the task is delaying
 * \param[in]   task_id     触发延时的任务 ID
 *
 * \note This function is declared but implementations are provided only when
 *       CFE_SIM_STEPPING is defined. When not defined, this becomes a no-op.
 */
void OS_PosixStepping_Hook_TaskDelay(uint32_t ms, osal_id_t task_id);

void OS_PosixStepping_Hook_TaskDelay_Complete(uint32_t ms, osal_id_t task_id);

/**
 * \brief Hook called at queue receive boundary
 *
 * Called when a task attempts to receive a message from a queue to allow stepping
 * implementations to manage message delivery timing in deterministic simulations.
 *
 * \note This function is declared but implementations are provided only when
 *       CFE_SIM_STEPPING is defined. When not defined, this becomes a no-op.
 */
void OS_PosixStepping_Hook_QueueReceive(const OS_object_token_t *token, int32 timeout);

void OS_PosixStepping_Hook_QueueReceive_Complete(const OS_object_token_t *token, int32 timeout, int32 return_code);

/**
 * \brief Hook called at binary semaphore take boundary
 *
 * Called when a task attempts to take a binary semaphore to allow stepping
 * implementations to manage synchronization points in deterministic simulations.
 *
 * \note This function is declared but implementations are provided only when
 *       CFE_SIM_STEPPING is defined. When not defined, this becomes a no-op.
 */
void OS_PosixStepping_Hook_BinSemTake(const OS_object_token_t *token, const struct timespec *timeout);

void OS_PosixStepping_Hook_BinSemTake_Complete(const OS_object_token_t *token, const struct timespec *timeout,
                                               int32 return_code);

#endif /* OS_POSIX_STEPPING_H */
