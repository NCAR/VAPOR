#!/bin/sh

ROOT_DIR=$(git rev-parse --show-toplevel)
HOOK_DIR=$ROOT_DIR/.git/hooks
ln -sf $ROOT_DIR/share/gitHooks/pre-push $HOOK_DIR/pre-push
