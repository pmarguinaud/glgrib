#!/bin/bash


set -x
set -e


unset LANG
unset LC_ALL
unset LANGUAGE

EMAIL=pmarguinaud@hotmail.com
NAME="Philippe Marguinaud"
#DEBIAN=debian:unstable
#DEBIAN=debian:latest
DEBIAN=ubuntu:20.04

sudo docker pull $DEBIAN

if [ ! -f "$HOME/.ssh/known_hosts" ]
then
  touch "$HOME/.ssh/known_hosts" 
fi

if [ ! -f "$HOME/.ssh/id_rsa" ]
then
  ssh-keygen -b 2048 -t rsa -f "$HOME/.ssh/id_rsa" -q -N ""
fi


SSHKEY=$(cat $HOME/.ssh/id_rsa.pub)
SHARED=$PWD/../$DEBIAN
TIMEZONE=$(cat /etc/timezone)

\rm -rf $SHARED
mkdir -p $SHARED

mkdir -p $SHARED/.ssh
cat > $SHARED/.ssh/authorized_keys << EOF
$SSHKEY
EOF

chmod 600 $SHARED/.ssh/authorized_keys

cat -> $SHARED/.bash_profile << EOF

export EMAIL=$EMAIL
unset LANG

set -o vi

EOF

cat -> $SHARED/.vimrc << EOF
:set mouse=
:set hlsearch
EOF

cat > $SHARED/glgrib-build.sh << EOF
#!/bin/bash

set -x
set -e

git config --global user.email "$EMAIL"
git config --global user.name "$NAME"
git config --global credential.helper cache
git config --global credential.helper 'cache --timeout=3600'

cd glgrib

./scripts/debian/tar.pl
rm -rf debian
cp -r packaging/debian debian
debuild -us -uc

EOF
 
cat > $SHARED/glgrib-install.sh << EOF
#!/bin/bash

set -x
set -e

dpkg-scanpackages -m . > Packages

sudo apt-get update

for pp in glgrib/debian/*.install
do
  pack=\$(basename \$pp .install)
  set +e
  sudo apt-get -y install \$pack
  set -e
done

EOF

cat > $SHARED/glgrib-uninstall.sh << EOF
#!/bin/bash

set -x
set -e

for pp in glgrib/debian/*.install
do
  pack=\$(basename \$pp .install)
  set +e
  sudo apt-get -y remove \$pack
  set -e
done

sudo apt-get autoremove -y 

EOF

cat > $SHARED/glgrib-test.sh << EOF
#!/bin/bash

set -x
set -e

glgrib --grid.on

EOF

cat > $SHARED/glgrib-boot.sh << EOF
#!/bin/bash

set -x
set -e

export DEBIAN_FRONTEND=noninteractive

kind=\$1

dir=\$(dirname \$0)

cd \$dir

ln -snf /usr/share/zoneinfo/$TIMEZONE /etc/localtime 
echo $TIMEZONE > /etc/timezone

apt-get -y update 
apt-get -y dist-upgrade
apt-get -y install git vim ssh screen sudo

if [ "x\$kind" = "xbuild" ]
then
  apt-get -y install debhelper-compat
  apt-get -y install \
    g++ make libcurl4-openssl-dev libeccodes-dev libegl-dev libglew-dev \
    libglfw3-dev libnetcdf-c++4-dev libpng-dev libreadline-dev libshp-dev \
    libsqlite3-dev libssl-dev libglm-dev build-essential devscripts debhelper \
    patchelf python3-dev libgbm-dev python3-setuptools dh-exec libimage-magick-perl \
    libdbi-perl libjson-perl libdbd-sqlite3-perl python3-pip
fi

if [ "x\$kind" = "xinstall" ]
then
  apt-get -y install dpkg-dev
  cat > /etc/apt/sources.list.d/glgrib.list << EOC
deb [trusted=yes] file:/home/$USER /
EOC

fi

echo 'X11UseLocalhost no' >> /etc/ssh/sshd_config

service ssh start

useradd $USER
chsh -s /bin/bash $USER

cat >> /etc/sudoers << EOC

$USER ALL=(root) NOPASSWD: /usr/bin/dpkg, NOPASSWD: /usr/bin/apt, NOPASSWD: /usr/bin/apt-get, NOPASSWD: /usr/bin/dpkg, NOPASSWD: /bin/su
EOC

EOF

cat > $SHARED/glgrib-clean.sh << EOF
#!/bin/bash

set -e
set -x

for f in *
do
  if [ -d "\$f" ]
  then
    \rm -rf "\$f"
  fi
done

EOF

chmod +x $SHARED/*.sh

declare -A IP

for kind in build install
do
  sudo docker \
    run -h glgrib_$kind -t -d \
    --name glgrib_$kind \
    --mount type=bind,src=$SHARED/,dst=/home/$USER \
    --mount type=bind,src=$PWD/,dst=/home/$USER/glgrib \
    $DEBIAN
  
  sudo docker exec glgrib_$kind /home/$USER/glgrib-boot.sh $kind
  
  IP[$kind]=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" glgrib_$kind)
  ssh-keygen -f "$HOME/.ssh/known_hosts" -R ${IP[$kind]}

  echo "${IP[$kind]}" > glgrib_$kind.txt
done

exit

SSH="ssh -o StrictHostKeyChecking=no -X"

# Build

$SSH ${IP[build]} ./glgrib-build.sh

# Install

$SSH ${IP[install]} ./glgrib-install.sh

# Test

$SSH ${IP[install]} ./glgrib-test.sh

# Uninstall

$SSH ${IP[install]} ./glgrib-uninstall.sh


exit

# apt list g++ make libcurl4-openssl-dev libeccodes-dev libegl-dev libglew-dev \
#   libglfw3-dev libnetcdf-c++4-dev libpng-dev libreadline-dev libshp-dev \
#   libsqlite3-dev libssl-dev libglm-dev  \
#   | perl -ne ^C,^(\S+)/.*now (\S+),o; print "$1 (>= $2)\n" ' 

apt list g++ make libcurl4-openssl-dev libeccodes-dev libegl-dev libglew-dev \
  libglfw3-dev libnetcdf-c++4-dev libpng-dev libreadline-dev libshp-dev \
  libsqlite3-dev libssl-dev libglm-dev  \
  | perl -e ' my @p; while (<>) { m,^(\S+)/.*now (\S+)-\d+\b,o; push @p, "$1 (>= $2)" if ($1 && $2) } print join (", ", @p) . "\n" ' 




