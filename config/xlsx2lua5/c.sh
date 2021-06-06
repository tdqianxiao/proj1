#!/bin/bash
find $1/xlsx -maxdepth 20 | xargs ls -Rd | grep ".xlsx$"
