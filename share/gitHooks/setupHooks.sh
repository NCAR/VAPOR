#!/bin/sh

ROOT_DIR=$(git rev-parse --show-toplevel)
HOOK_DIR=$ROOT_DIR/.git/hooks
HOOK=$HOOK_DIR/pre-push
ln -sf $ROOT_DIR/share/gitHooks/pre-push $HOOK

if [ -e "$HOOK" ]
then
    echo "pre-push hook installed in $HOOK_DIR"
else
    echo "Failure: Unable to create sym-link in $HOOK_DIR"
fi
