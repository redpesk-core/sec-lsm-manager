/*
 * Copyright (C) 2020 IoT.bzh Company
 * Author: Arthur Guyader <arthur.guyader@iot.bzh>
 *
 * $RP_BEGIN_LICENSE$
 * Commercial License Usage
 *  Licensees holding valid commercial IoT.bzh licenses may use this file in
 *  accordance with the commercial license agreement provided with the
 *  Software or, alternatively, in accordance with the terms contained in
 *  a written agreement between you and The IoT.bzh Company. For licensing terms
 *  and conditions see https://www.iot.bzh/terms-conditions. For further
 *  information use the contact form at https://www.iot.bzh/contact.
 *
 * GNU General Public License Usage
 *  Alternatively, this file may be used under the terms of the GNU General
 *  Public license version 3. This license is as published by the Free Software
 *  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
 *  of this file. Please review the following information to ensure the GNU
 *  General Public License requirements will be met
 *  https://www.gnu.org/licenses/gpl-3.0.html.
 * $RP_END_LICENSE$
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
