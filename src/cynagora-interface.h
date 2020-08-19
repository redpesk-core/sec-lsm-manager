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

#ifndef SECURITY_MANAGER_CYNAGORA_INTERFACE_H
#define SECURITY_MANAGER_CYNAGORA_INTERFACE_H

#include "policies.h"

/**
 * @brief Drop old policies of cynagora for an id (client)
 *
 * @param[in] cynagora cynagora admin client
 * @param[in] client the id of the application for which to remove permissions
 * @return 0 in case of success or a negative -errno value
 */
int cynagora_drop_policies(cynagora_t *cynagora, const char *client) __wur;

/**
 * @brief Define new permissions in cynagora
 *
 * @param[in] cynagora cynagora admin client
 * @param[in] policies array of policies_t
 * @return 0 in case of success or a negative -errno value
 */
int cynagora_set_policies(cynagora_t *cynagora, const policies_t *policies) __wur;
#endif