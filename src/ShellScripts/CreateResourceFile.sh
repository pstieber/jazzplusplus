#******************************************************************************
# The JAZZ++ Midi Sequencer
#
# Copyright (C) 2016 Peter J. Stieber, all rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#******************************************************************************

#******************************************************************************
#******************************************************************************
ReadVersion()
{
  FILE_NAME=../JazzPlusPlusVersion/Versions.txt

  if [ -f "${FILE_NAME}" ]
  then
    # Link file descriptor 10 with stdin.
    exec 10<&0

    # Replace stdin with a file.
    exec < "${FILE_NAME}"

    read LINE
    MAJOR_VERSION=$(echo $LINE | cut -d' ' -f1)
    read LINE
    MINOR_VERSION=$(echo $LINE | cut -d' ' -f1)
    read LINE
    BUILD_NUMBER=$(echo $LINE | cut -d' ' -f1)

    # Restore stdin from file descriptor 10 and close file descriptor 10.
    exec 0<&10 10<&-
  fi
}

#******************************************************************************
#******************************************************************************
ReadVersion

COMMIT_COUNT=$(git log --pretty=format:"" | wc -l | sed 's/\s\+//g')
BRANCH_NAME=$(git symbolic-ref --short HEAD)
echo Branch: $BRANCH_NAME
ESCAPED_BRANCH_NAME=$(echo $BRANCH_NAME | sed 's/\//\\\//g')

echo Jazz++ Version: ${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER}.${COMMIT_COUNT}

sed 's/MAJOR_VERSION/'$(echo ${MAJOR_VERSION})'/' JazzPlusPlusResources.tpl | \
sed 's/BRANCH_NAME/'$(echo ${ESCAPED_BRANCH_NAME})'/' | \
sed 's/COMMIT_COUNT/'$(echo ${COMMIT_COUNT})'/' | \
sed 's/MINOR_VERSION/'$(echo ${MINOR_VERSION})'/' | \
sed 's/BUILD_NUMBER/'$(echo ${BUILD_NUMBER})'/' \
> JazzPlusPlus.rc
