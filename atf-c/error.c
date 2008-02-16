/*
 * Automated Testing Framework (atf)
 *
 * Copyright (c) 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atf-c/error.h"
#include "atf-c/sanity.h"

/* ---------------------------------------------------------------------
 * Auxiliary functions.
 * --------------------------------------------------------------------- */

static
void
error_format(const atf_error_t err, char *buf, size_t buflen)
{
    PRE(err != NULL);
    snprintf(buf, buflen, "Error '%s'", err->m_type);
}

static
bool
error_init(atf_error_t err, const char *type, void *data, size_t datalen,
           void (*format)(const atf_error_t, char *, size_t))
{
    bool ok;

    PRE(data != NULL || datalen == 0);
    PRE(datalen != 0 || data == NULL);

    atf_object_init(&err->m_object);

    err->m_free = false;
    err->m_type = type;
    err->m_format = (format == NULL) ? error_format : format;

    ok = true;
    if (data == NULL) {
        err->m_data = NULL;
    } else {
        err->m_data = malloc(datalen);
        if (err->m_data == NULL)
            ok = false;
        else
            memcpy(err->m_data, data, datalen);
    }

    return ok;
}

/* ---------------------------------------------------------------------
 * The "atf_error" type.
 * --------------------------------------------------------------------- */

atf_error_t
atf_error_new(const char *type, void *data, size_t datalen,
              void (*format)(const atf_error_t, char *, size_t))
{
    atf_error_t err;

    PRE(data != NULL || datalen == 0);
    PRE(datalen != 0 || data == NULL);

    err = malloc(sizeof(*err));
    if (err == NULL)
        err = atf_no_memory_error();
    else {
        if (!error_init(err, type, data, datalen, format)) {
            free(err);
            err = atf_no_memory_error();
        } else
            err->m_free = true;
    }

    INV(err != NULL);
    return err;
}

void
atf_error_free(atf_error_t err)
{
    bool freeit;

    PRE(err != NULL);

    freeit = err->m_free;

    if (err->m_data != NULL)
        free(err->m_data);

    atf_object_fini(&err->m_object);

    if (freeit)
        free(err);
}

atf_error_t
atf_no_error(void)
{
    return NULL;
}

bool
atf_is_error(const atf_error_t err)
{
    return err != NULL;
}

bool
atf_error_is(const atf_error_t err, const char *type)
{
    PRE(err != NULL);

    return strcmp(err->m_type, type) == 0;
}

const void *
atf_error_data(const atf_error_t err)
{
    PRE(err != NULL);

    return err->m_data;
}

void
atf_error_format(const atf_error_t err, char *buf, size_t buflen)
{
    PRE(err != NULL);
    err->m_format(err, buf, buflen);
}

/* ---------------------------------------------------------------------
 * Common error types.
 * --------------------------------------------------------------------- */

/*
 * The "libc" error.
 */

struct atf_libc_error_data {
    int m_errno;
    const char *m_what;
};
typedef struct atf_libc_error_data atf_libc_error_data_t;

static
void
libc_format(const atf_error_t err, char *buf, size_t buflen)
{
    const struct atf_libc_error_data *data;

    PRE(atf_error_is(err, "libc"));

    data = atf_error_data(err);
    snprintf(buf, buflen, "%s: %s", data->m_what, strerror(data->m_errno));
}

atf_error_t
atf_libc_error(int syserrno, const char *what)
{
    atf_error_t err;
    atf_libc_error_data_t data;

    data.m_errno = syserrno;
    data.m_what = what;

    err = atf_error_new("libc", &data, sizeof(data), libc_format);

    return err;
}

int
atf_libc_error_code(const atf_error_t err)
{
    const struct atf_libc_error_data *data;

    PRE(atf_error_is(err, "libc"));

    data = atf_error_data(err);

    return data->m_errno;
}

/*
 * The "no_memory" error.
 */

static bool no_memory_error_inited = false;
static struct atf_error no_memory_error;

static
void
no_memory_format(const atf_error_t err, char *buf, size_t buflen)
{
    PRE(atf_error_is(err, "no_memory"));

    snprintf(buf, buflen, "Not enough memory");
}

atf_error_t
atf_no_memory_error(void)
{
    if (!no_memory_error_inited) {
        error_init(&no_memory_error, "no_memory", NULL, 0,
                   no_memory_format);
        no_memory_error_inited = true;
    }

    return &no_memory_error;
}
