#!/bin/bash


set -x
set -e


unset LANG
unset LC_ALL
unset LANGUAGE

EMAIL=pmarguinaud@hotmail.com
NAME="Philippe Marguinaud"
DEBIAN=latest
VERSION=1.0
PACKVER=1

sudo docker pull debian:$DEBIAN

SSHKEY=$(cat $HOME/.ssh/id_rsa.pub)

SHARED=$PWD/debuild

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

git clone -b debian https://github.com/pmarguinaud/glgrib.git
cd glgrib
./scripts/debian.pl --create-archive --create-directory 
./scripts/debian.pl --compile
EOF
 
cat > $SHARED/glgrib-install.sh << EOF
#!/bin/bash

set -x
set -e

for pack in data doc test bin
do
  set +e
  sudo dpkg -i ./glgrib-\${pack}_${VERSION}-${PACKVER}_amd64.deb  
  set -e
done

sudo apt-get install -y -f

EOF

cat > $SHARED/glgrib-uninstall.sh << EOF
#!/bin/bash

set -x
set -e

for pack in bin data doc test
do
  sudo apt-get remove -y glgrib-\$pack
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

kind=\$1

dir=\$(dirname \$0)

cd \$dir

apt-get -y update 
apt-get -y dist-upgrade
apt-get -y install git vim ssh screen sudo

if [ "x\$kind" = "xbuild" ]
then
  apt-get -y install \
    g++ make libcurl4-openssl-dev libeccodes-dev libegl-dev libglew-dev \
    libglfw3-dev libnetcdf-c++4-dev libpng-dev libreadline-dev libshp-dev \
    libsqlite3-dev libssl-dev libglm-dev build-essential devscripts debhelper \
    patchelf
fi

echo 'X11UseLocalhost no' >> /etc/ssh/sshd_config

service ssh start

useradd $USER
chsh -s /bin/bash $USER

cat >> /etc/sudoers << EOC

$USER ALL=(root) NOPASSWD: /usr/bin/dpkg, NOPASSWD: /usr/bin/apt, NOPASSWD: /usr/bin/apt-get, NOPASSWD: /usr/bin/dpkg
EOC

EOF

chmod +x $SHARED/*.sh

declare -A IP

for kind in build install
do
  sudo docker \
    run -h debian_glgrib_$kind -t -d \
    --name debian_glgrib_$kind \
    --mount type=bind,src=$SHARED/,dst=/home/$USER \
    debian:$DEBIAN
  
  sudo docker exec debian_glgrib_$kind /home/$USER/glgrib-boot.sh $kind
  
  IP[$kind]=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" debian_glgrib_$kind)
  ssh-keygen -f "$HOME/.ssh/known_hosts" -R ${IP[$kind]}

  echo "${IP[$kind]}" > debian_glgrib_$kind.txt
done


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




