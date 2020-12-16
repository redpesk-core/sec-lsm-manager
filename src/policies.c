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

#include "policies.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

/***********************/
/*** PRIVATE METHODS ***/
/***********************/

/**
 * @brief Free cynagora key
 * The pointer is not free
 *
 * @param key cynagora_key handler
 */
static void free_cynagora_key(cynagora_key_t *key) {
    if (key) {
        free((void *)key->client);
        key->client = NULL;
        free((void *)key->permission);
        key->permission = NULL;
        free((void *)key->session);
        key->session = NULL;
        free((void *)key->user);
        key->user = NULL;
    }
}

/**
 * @brief Free cynagora value
 * The pointer is not free
 *
 * @param value cynagora_value handler
 */
static void free_cynagora_value(cynagora_value_t *value) {
    if (value) {
        free((void *)value->value);
        value->value = NULL;
    }
}

/**
 * @brief Copy cynagora key (alloc each parameters)
 *
 * @param dest The cynagora key destination handler (need to be allocated)
 * @param src The source cynagora key
 * @return 0 in case of success or a negative -errno value
 */
static int copy_cynagora_key(cynagora_key_t *dest, const cynagora_key_t *src) {
    if (!dest) {
        ERROR("dest undefined");
        return -EINVAL;
    } else if (!src) {
        ERROR("src undefined");
        return -EINVAL;
    }

    memset(dest, 0, sizeof(*dest));

    if (src->client) {
        dest->client = strdup(src->client);
        if (dest->client == NULL) {
            ERROR("strdup client");
            free_cynagora_key(dest);
            return -ENOMEM;
        }
    }

    if (src->permission) {
        dest->permission = strdup(src->permission);
        if (dest->permission == NULL) {
            ERROR("strdup permission");
            free_cynagora_key(dest);
            return -ENOMEM;
        }
    }

    if (src->session) {
        dest->session = strdup(src->session);
        if (dest->session == NULL) {
            ERROR("strdup session");
            free_cynagora_key(dest);
            return -ENOMEM;
        }
    }

    if (src->user) {
        dest->user = strdup(src->user);
        if (dest->user == NULL) {
            ERROR("strdup user");
            free_cynagora_key(dest);
            return -ENOMEM;
        }
    }

    return 0;
}

/**
 * @brief Copy cynagora value (alloc value)
 *
 * @param dest The cynagora value destination handler (need to be allocated)
 * @param src The source cynagora value
 * @return 0 in case of success or a negative -errno value
 */
static int copy_cynagora_value(cynagora_value_t *dest, const cynagora_value_t *src) {
    if (!dest) {
        ERROR("dest undefined");
        return -EINVAL;
    } else if (!src) {
        ERROR("src undefined");
        return -EINVAL;
    }

    memset(dest, 0, sizeof(*dest));

    if (src->value) {
        dest->value = strdup(src->value);
        if (dest->value == NULL) {
            ERROR("strdup value");
            return -ENOMEM;
        }
    }

    dest->expire = src->expire;
    return 0;
}

/**
 * @brief Free cynagora key and cynagora value
 * The pointer is not free
 * @param policy policy handler
 */
static void free_policy(policy_t *policy) {
    if (policy) {
        free_cynagora_key(&(policy->k));
        free_cynagora_value(&(policy->v));
    }
}

/**********************/
/*** PUBLIC METHODS ***/
/**********************/

/* see policies.h */
int init_policies(policies_t *policies) {
    if (!policies) {
        ERROR("policies undefined");
        return -EINVAL;
    }
    policies->size = 0;
    policies->policies = NULL;
    return 0;
}

/* see policies.h */
void free_policies(policies_t *policies) {
    if (policies) {
        if (policies->size) {
            for (size_t i = 0; i < policies->size; i++) {
                free_policy(policies->policies + i);
            }
        }
        policies->size = 0;
        free(policies->policies);
        policies->policies = NULL;
    }
}

/* see policies.h */
int policies_add_policy(policies_t *policies, const cynagora_key_t *k, const cynagora_value_t *v) {
    if (!policies) {
        ERROR("policies undefined");
        return -EINVAL;
    } else if (!k) {
        ERROR("k undefined");
        return -EINVAL;
    } else if (!v) {
        ERROR("v undefined");
        return -EINVAL;
    }

    if (policies->size == 0) {
        policies->policies = (policy_t *)malloc(sizeof(policy_t));
        if (policies->policies == NULL) {
            ERROR("malloc policy_t");
            return -ENOMEM;
        }
    } else {
        policy_t *policies_tmp = (policy_t *)realloc(policies->policies, sizeof(policy_t) * (policies->size + 1));
        if (policies_tmp == NULL) {
            ERROR("realloc policy_t");
            free_policies(policies);
            return -ENOMEM;
        }
        policies->policies = policies_tmp;
    }

    int rc = copy_cynagora_key(&(policies->policies[policies->size].k), k);
    if (rc < 0) {
        ERROR("copy_cynagora_key");
        free_policies(policies);
        return rc;
    }

    rc = copy_cynagora_value(&(policies->policies[policies->size].v), v);
    if (rc < 0) {
        ERROR("copy_cynagora_value");
        free_cynagora_key(&(policies->policies[policies->size].k));
        free_policies(policies);
        return rc;
    }

    policies->size++;

    return 0;
}
