#!/usr/bin/env bash

set -u

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly PROJECT_DIR
readonly PROGRAM="${PROJECT_DIR}/bin/fix-hostfiles"

tests_run=0
tests_failed=0

pass() {
    tests_run=$((tests_run + 1))
    printf 'ok %d - %s\n' "$tests_run" "$1"
}

fail() {
    tests_run=$((tests_run + 1))
    tests_failed=$((tests_failed + 1))
    printf 'not ok %d - %s\n' "$tests_run" "$1"
}

assert_command() {
    local description="$1"
    shift

    if "$@"; then
        pass "$description"
    else
        fail "$description"
    fi
}

make_fixture() {
    local fixture_dir="$1"

    mkdir -p "${fixture_dir}/bin" "${fixture_dir}/etc/hblock"
    printf '127.0.0.1 localhost\n0.0.0.0 ads.example.com\n0.0.0.0 notads.example.com\n' >"${fixture_dir}/etc/hosts"
    : >"${fixture_dir}/etc/hblock/allow.list"
    printf '#!/bin/sh\nexit 0\n' >"${fixture_dir}/bin/hblock"
    chmod +x "${fixture_dir}/bin/hblock"
}

run_fixture() {
    local fixture_dir="$1"
    shift

    FIX_HOSTFILES_ETC_DIR="${fixture_dir}/etc" \
        FIX_HOSTFILES_HBLOCK_DIR="${fixture_dir}/etc/hblock" \
        PATH="${fixture_dir}/bin:/usr/bin:/bin" \
        "$PROGRAM" "$@"
}

test_help() {
    "$PROGRAM" --help 2>&1 | grep -q '^Usage:'
}

test_missing_action() {
    ! "$PROGRAM" >/dev/null 2>&1
}

test_conflicting_actions() {
    ! "$PROGRAM" --flush prep >/dev/null 2>&1
}

test_invalid_dns_name() {
    ! "$PROGRAM" --add invalid >/dev/null 2>&1
}

test_prep() {
    local fixture_dir="$1"

    run_fixture "$fixture_dir" prep >/dev/null &&
        cmp -s "${fixture_dir}/etc/hosts" "${fixture_dir}/etc/hosts-ORIG"
}

test_verbose_prep() {
    local fixture_dir="$1"
    local output

    output="$(run_fixture "$fixture_dir" --verbose prep)" &&
        grep -q 'Source file details:' <<<"$output" &&
        grep -q 'Permissions:' <<<"$output" &&
        grep -q 'Original hosts backup details:' <<<"$output"
}

test_restore() {
    local fixture_dir="$1"

    printf 'original hosts\n' >"${fixture_dir}/etc/hosts-ORIG"
    printf 'y\n' | run_fixture "$fixture_dir" restore >/dev/null &&
        grep -qFx 'original hosts' "${fixture_dir}/etc/hosts"
}

test_add_dns_name() {
    local fixture_dir="$1"

    run_fixture "$fixture_dir" --add ads.example.com >/dev/null &&
        grep -qFx 'ads.example.com' "${fixture_dir}/etc/hblock/allow.list" &&
        ! grep -qF ' ads.example.com' "${fixture_dir}/etc/hosts" &&
        grep -qF ' notads.example.com' "${fixture_dir}/etc/hosts" &&
        [[ -f "${fixture_dir}/etc/hosts.bak" ]]
}

test_add_dns_name_is_idempotent() {
    local fixture_dir="$1"

    run_fixture "$fixture_dir" --add ads.example.com >/dev/null &&
        run_fixture "$fixture_dir" --add ads.example.com >/dev/null &&
        [[ $(grep -cFx 'ads.example.com' "${fixture_dir}/etc/hblock/allow.list") -eq 1 ]]
}

fixture_root="$(mktemp -d "${TMPDIR:-/tmp}/fix-hostfiles-c-test.XXXXXX")"
trap 'rm -rf "$fixture_root"' EXIT

printf '1..9\n'
assert_command 'help succeeds' test_help
assert_command 'a missing action fails' test_missing_action
assert_command 'conflicting actions fail' test_conflicting_actions
assert_command 'an invalid DNS name fails' test_invalid_dns_name

make_fixture "${fixture_root}/prep"
assert_command 'prep backs up the hosts file' test_prep "${fixture_root}/prep"

make_fixture "${fixture_root}/verbose"
assert_command 'verbose prep reports before-and-after metadata' test_verbose_prep "${fixture_root}/verbose"

make_fixture "${fixture_root}/restore"
assert_command 'restore reinstates the original hosts file' test_restore "${fixture_root}/restore"

make_fixture "${fixture_root}/add"
assert_command 'add updates the allow list and removes only the exact host' test_add_dns_name "${fixture_root}/add"

make_fixture "${fixture_root}/idempotent"
assert_command 'adding the same DNS name twice is idempotent' test_add_dns_name_is_idempotent "${fixture_root}/idempotent"

(( tests_failed == 0 ))
