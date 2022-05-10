#!/bin/bash
cd "$(dirname "$0")"

if [[ "$(docker images -q markdownlatex:latest 2> /dev/null)" == "" ]]; then
  echo "markdownlatex:latest IMAGE BUILD STARTED"
  docker build -t markdownlatex:latest .
else
 echo "markdownlatex:latest IMAGE EXISTS; NO BUILD REQUIRED"
fi


docker run -i --rm -v "$(pwd)":/var/thesis markdownlatex
