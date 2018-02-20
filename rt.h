/**
 * General glucose run-time stuff.
 */

#pragma once

static char * glc_untag(const char * tagged) {
    return (char *) (((size_t) tagged) & -16); // TODO?
}

static const char * glc_copytag(const char * from, const char * to) {
    return (char *) (((size_t) to) | ((size_t) from & 15)); // TODO?
}
