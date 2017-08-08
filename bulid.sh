#!/bin/bash

echo "============ hexo clean begin ============"
hexo clean
echo "============ hexo generate begin ============"
hexo generate
echo "============ hexo deploy begin ============"
hexo deploy

echo "============ DONE ============"

