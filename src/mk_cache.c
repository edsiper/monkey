/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Server
 *  ==================
 *  Copyright 2001-2014 Monkey Software LLC <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <monkey/mk_iov.h>
#include <monkey/mk_cache.h>
#include <monkey/mk_request.h>
#include <monkey/mk_string.h>
#include <monkey/mk_config.h>
#include <monkey/mk_macros.h>
#include <monkey/mk_utils.h>
#include <monkey/mk_vhost.h>

pthread_key_t mk_utils_error_key;

__thread struct mk_iov *worker_cache_iov_header;
__thread mk_ptr_t *worker_cache_header_cl;
__thread mk_ptr_t *worker_cache_header_lm;
__thread mk_ptr_t *worker_cache_header_ka;
__thread mk_ptr_t *worker_cache_header_ka_max;
__thread struct tm *worker_cache_gmtime;
__thread struct mk_gmt_cache *worker_cache_gmtext;

/* This function is called when a thread is created */
void mk_cache_worker_init()
{
    char *cache_error;

    /* Cache header request -> last modified */
    worker_cache_header_lm = mk_mem_malloc_z(sizeof(mk_ptr_t));
    worker_cache_header_lm->data = mk_mem_malloc_z(32);
    worker_cache_header_lm->len = -1;

    /* Cache header request -> content length */
    worker_cache_header_cl = mk_mem_malloc_z(sizeof(mk_ptr_t));
    worker_cache_header_cl->data = mk_mem_malloc_z(MK_UTILS_INT2MKP_BUFFER_LEN);
    worker_cache_header_cl->len = -1;

    /* Cache header response -> keep-alive */
    worker_cache_header_ka = mk_mem_malloc_z(sizeof(mk_ptr_t));
    mk_string_build(&worker_cache_header_ka->data, &worker_cache_header_ka->len,
                    "Keep-Alive: timeout=%i, max=",
                    config->keep_alive_timeout);

    /* Cache header response -> max=%i */
    worker_cache_header_ka_max = mk_mem_malloc_z(sizeof(mk_ptr_t));
    worker_cache_header_ka_max->data = mk_mem_malloc_z(64);
    worker_cache_header_ka_max->len  = 0;

    /* Cache iov header struct */
    worker_cache_iov_header = mk_iov_create(32, 0);

    /* Cache gmtime buffer */
    worker_cache_gmtime = mk_mem_malloc(sizeof(struct tm));

    /* Cache the most used text representations of utime2gmt */
    worker_cache_gmtext = mk_mem_malloc_z(sizeof(struct mk_gmt_cache) * MK_GMT_CACHES);

    /* Cache buffer for strerror_r(2) */
    cache_error = mk_mem_malloc(MK_UTILS_ERROR_SIZE);
    pthread_setspecific(mk_utils_error_key, (void *) cache_error);

    /* Virtual hosts: initialize per thread-vhost data */
    mk_vhost_fdt_worker_init();
}

void mk_cache_worker_exit()
{
    char *cache_error;

    /* Cache header request -> last modified */
    mk_mem_free(worker_cache_header_lm->data);
    mk_mem_free(worker_cache_header_lm);

    /* Cache header request -> content length */
    mk_mem_free(worker_cache_header_cl->data);
    mk_mem_free(worker_cache_header_cl);

    /* Cache header response -> keep-alive */
    mk_mem_free(worker_cache_header_ka->data);
    mk_mem_free(worker_cache_header_ka);

    /* Cache header response -> max=%i */
    mk_mem_free(worker_cache_header_ka_max->data);
    mk_mem_free(worker_cache_header_ka_max);

    /* Cache iov header struct */
    mk_iov_free(worker_cache_iov_header);

    /* Cache gmtime buffer */
    mk_mem_free(worker_cache_gmtime);

    /* Cache the most used text representations of utime2gmt */
    mk_mem_free(worker_cache_gmtext);

    /* Cache buffer for strerror_r(2) */
    cache_error = pthread_getspecific(mk_utils_error_key);
    mk_mem_free(cache_error);
}
