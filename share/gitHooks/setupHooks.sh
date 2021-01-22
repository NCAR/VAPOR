#!/bin/sh

ROOT_DIR=$(git rev-parse --show-toplevel)
HOOK_DIR=$ROOT_DIR/.git/hooks
ln -s $ROOT_DIR/share/gitHooks/pre-commit $HOOK_DIR/pre-commit
