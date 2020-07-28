#!/bin/bash

HOOK_DIR=$(git rev-parse --show-toplevel)/.git/hooks
ln -s $(pwd)/pre-commit $HOOK_DIR/pre-commit
