#!/bin/sh

packageDir="package"

arch=$(getarch)
arch=${arch//_/-}

mkdir -p $packageDir
cp .PackageInfo ${packageDir}/.PackageInfo

mkdir -p ${packageDir}/apps/ALEditor
cp objects.${arch}-release/ALEditor ${packageDir}/apps/ALEditor/ALEditor

deskbarEntryDir=data/deskbar/menu/Applications
mkdir -p ${packageDir}/${deskbarEntryDir}
ln -s -f -t ${packageDir}/${deskbarEntryDir} ../../../../apps/ALEditor/ALEditor

# copy development headers
headersALE="LayoutArchive.h"
headersALM="ALMGroup.h ALMLayout.h ALMLayoutBuilder.h Area.h Column.h Row.h Tab.h"
headersLinearSpec="Constraint.h LinearProgrammingTypes.h LinearSpec.h Summand.h Variable.h"

mkdir -p ${packageDir}/develop/headers/ale
for file in $headersALE
do
	cp headers/editor/${file} ${packageDir}/develop/headers/ale/${file}
done

mkdir -p ${packageDir}/develop/headers/alm
for file in $headersALM
do
	cp headers/haiku_private/alm/${file} ${packageDir}/develop/headers/alm/${file}
done

mkdir -p ${packageDir}/develop/headers/linearspec
for file in $headersLinearSpec
do
	cp headers/haiku_private/linearspec/${file} ${packageDir}/develop/headers/linearspec/${file}
done

# copy example
mkdir -p ${packageDir}/data/ale/example
cp example/* ${packageDir}/data/ale/example/

# create package
package create -C ${packageDir}/ ALEditor.hpkg
