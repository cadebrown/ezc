#!/bin/bash

cd src
pydoc -w ./
cd ..
mv ./src/*.html ./docs/