#!/bin/sh

ROOT_DIR=$(git rev-parse --show-toplevel)
HOOK_DIR=$ROOT_DIR/.git/hooks
ln -sf $ROOT_DIR/share/gitHooks/pre-receive $HOOK_DIR/pre-receive
