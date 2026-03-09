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
void OS_PosixStepping_Hook_TaskDelay(uint32_t ms)
{
    /* Stub implementation - no-op */
    (void)ms;
}

/**
 * \brief Hook called at queue receive boundary
 *
 * Stub implementation that is called when a task attempts to receive a message from a queue.
 * Real implementations can use this to manage message delivery timing in deterministic simulations.
 */
void OS_PosixStepping_Hook_QueueReceive(void)
{
    /* Stub implementation - no-op */
}

/**
 * \brief Hook called at binary semaphore take boundary
 *
 * Stub implementation that is called when a task attempts to take a binary semaphore.
 * Real implementations can use this to manage synchronization points in deterministic simulations.
 */
void OS_PosixStepping_Hook_BinSemTake(void)
{
    /* Stub implementation - no-op */
}

#endif /* CFE_SIM_STEPPING */
