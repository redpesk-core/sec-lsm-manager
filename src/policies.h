/*
 * Copyright (C) 2020 "IoT.bzh"
 * Author Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SECURITY_MANAGER_POLICIES_H
#define SECURITY_MANAGER_POLICIES_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifndef SIMULATE_CYNAGORA
#include <cynagora.h>
#else
#include "simulation/cynagora/cynagora.h"
#endif

#define SELECT_ALL "#"
#define INSERT_ALL "*"
#define AUTHORIZED "yes"

/**
 * @brief Structure of policy
 * policy contain permission that will be send to cynagora
 *
 */
typedef struct policy {
    cynagora_key_t k;
    cynagora_value_t v;
} policy_t;

/**
 * @brief Structure of policies
 * policies contains several policy
 *
 */
typedef struct policies {
    policy_t *policies;
    size_t size;
} policies_t;

/**
 * @brief Initialize the fields 'size' and 'policies'
 *
 * @param[in] policies The policies handler
 * @return 0 in case of success or a negative -errno value
 */
int init_policies(policies_t *policies) __wur;

/**
 * @brief[in] Free policies that have been added
 * The pointer is not free
 * @param policies The policies handler
 */
void free_policies(policies_t *policies);

/**
 * @brief Add a policy to policies struct
 *
 * @param[in] policies The policies handler
 * @param[in] cynagora_key_t The cynagora key to add
 * @param[in] cynagora_value_t The cynagora value to add
 * @return 0 in case of success or a negative -errno value
 */
int policies_add_policy(policies_t *policies, const cynagora_key_t *k, const cynagora_value_t *v) __wur;

#endif