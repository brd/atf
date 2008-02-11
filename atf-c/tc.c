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

#include <stdarg.h>
#include <stdbool.h>

#include "sanity.h"
#include "tc.h"

const int atf_tcr_passed = 0;
const int atf_tcr_failed = 1;
const int atf_tcr_skipped = 2;

struct atf_tcr temporary_hack;

inline
bool
status_allows_reason(int status)
{
    return status == atf_tcr_failed || status == atf_tcr_skipped;
}

void
atf_tcr_init(struct atf_tcr *tcr, int status)
{
    PRE(!status_allows_reason(status));
    tcr->atcr_status = status;
    atf_dynstr_init(&tcr->atcr_reason);
}

int
atf_tcr_init_reason(struct atf_tcr *tcr, int status, const char *reason,
                    va_list ap)
{
    int ret = 0;

    PRE(status_allows_reason(status));

    tcr->atcr_status = status;
    ret = atf_dynstr_init_ap(&tcr->atcr_reason, reason, ap);

    return ret;
}

void
atf_tcr_fini(struct atf_tcr *tcr)
{
    atf_dynstr_fini(&tcr->atcr_reason);
}

int
atf_tcr_get_status(const struct atf_tcr *tcr)
{
    return tcr->atcr_status;
}

const char *
atf_tcr_get_reason(const struct atf_tcr *tcr)
{
    PRE(status_allows_reason(tcr->atcr_status));
    return atf_dynstr_cstring(&tcr->atcr_reason);
}

int
atf_tc_run(const struct atf_tc *tc, struct atf_tcr *tcr)
{
    atf_tcr_init(&temporary_hack, atf_tcr_passed);
    tc->atc_body(tc);
    *tcr = temporary_hack;
    return 0;
}

void
atf_tc_fail(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    atf_tcr_init_reason(&temporary_hack, atf_tcr_failed, fmt, ap);
    va_end(ap);
}

void
atf_tc_pass(void)
{
    atf_tcr_init(&temporary_hack, atf_tcr_passed);
}

void
atf_tc_skip(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    atf_tcr_init_reason(&temporary_hack, atf_tcr_skipped, fmt, ap);
    va_end(ap);
}

void
atf_tc_set_var(const char *var, const char *value, ...)
{
}
