#!/bin/bash


set -x
set -e

EMAIL=pmarguinaud@hotmail.com
NAME="Philippe Marguinaud"
DEBIAN=latest
VERSION=1.0
PACKVER=1

if [ 1 -eq 1 ]
then
sudo docker pull debian:$DEBIAN

SSHKEY=$(cat $HOME/.ssh/id_rsa.pub)

cat > boot.sh << EOF
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
mkdir -p /home/$USER/.ssh
chown -R $USER:users /home/$USER

cat > /home/$USER/.ssh/authorized_keys << EOC
$SSHKEY
EOC

chmod 600 /home/$USER/.ssh/authorized_keys

cat -> /home/$USER/.bash_profile << EOC

export EMAIL=$EMAIL
unset LANG

set -o vi

EOC

cat -> /home/$USER/.vimrc << EOC
:set mouse=
:set hlsearch
EOC

chown $USER:users /home/$USER/.ssh/authorized_keys /home/$USER/.bash_profile /home/$USER/.vimrc

cat >> /etc/sudoers << EOC

$USER ALL=(root) NOPASSWD: /usr/bin/dpkg, NOPASSWD: /usr/bin/apt, NOPASSWD: /usr/bin/apt-get
EOC

if [ "x\$kind" = "xbuild" ]
then
  cd /home/$USER
  cat > glgrib-build.sh << EOC
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
EOC
 
  chown $USER:users glgrib-*.sh
  chmod +x glgrib-*.sh

fi

if [ "x\$kind" = "xinstall" ]
then
  cd /home/$USER
  cat > glgrib-install.sh << EOC
#!/bin/bash

set -x
set -e

for pack in data bin doc test
do
  sudo apt-get install -y  ./glgrib-\\\$pack.0-1_amd64.deb  
done

EOC

  cat > glgrib-uninstall.sh << EOC
#!/bin/bash

set -x
set -e

for pack in bin data doc test
do
  sudo apt-get remove -y glgrib-\\$pack
done
sudo apt-get autoremove -y 

EOC

  cat > glgrib-test.sh << EOC
#!/bin/bash

set -x
set -e

glgrib --grid.on

EOC

  chown $USER:users glgrib-*.sh
  chmod +x glgrib-*.sh

fi

EOF

chmod +x boot.sh

declare -A IP

for kind in build install
do
  sudo docker run -h debian_glgrib_$kind -t -d --name debian_glgrib_$kind debian:$DEBIAN
  
  sudo docker cp boot.sh debian_glgrib_$kind:/root/boot.sh
  
  sudo docker exec debian_glgrib_$kind /root/boot.sh $kind
  
  IP[$kind]=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" debian_glgrib_$kind)
  ssh-keygen -f "$HOME/.ssh/known_hosts" -R ${IP[$kind]}

  echo "${IP[$kind]}" > debian_glgrib_$kind.txt
done

fi

declare -A IP

for kind in build install
do
  IP[$kind]=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" debian_glgrib_$kind)
done

SSH="ssh -o StrictHostKeyChecking=no -X"
SCP="scp -o StrictHostKeyChecking=no"


if [ 1 -eq 1 ]
then

# Build

$SSH ${IP[build]} ./glgrib-build.sh

fi

# Copy deb & install

for pack in bin doc data test
do
  $SCP \
    ${IP[build]}:glgrib-${pack}_1.0-1_amd64.deb \
    ${IP[install]}:glgrib-${pack}_1.0-1_amd64.deb
done

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




