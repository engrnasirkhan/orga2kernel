#!/bin/sh
DIRS=ej*

make -C lib clean; make -C lib
for i in $DIRS; do
	make -C $i clean
	make -C $i
done

[ -f utils/incrustar ] || make -C utils
