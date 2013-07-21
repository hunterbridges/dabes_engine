#!/bin/bash
THISDIR=`pwd`
cd resource/scripts
ldoc --config $THISDIR/etc/config.ld -f markdown -d ~/Coding/dabes_engine_pages/scripting/ dabes/
