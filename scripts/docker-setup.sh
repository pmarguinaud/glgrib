#!/bin/bash


set -x
set -e

UBUNTU=20.04
VERSION=1.0-1

if [ 1 -eq 1 ]
then
sudo docker pull ubuntu:$UBUNTU

SSHKEY=$(cat $HOME/.ssh/id_rsa.pub)

cat > glgrib.sh << EOF
#!/bin/bash

set -x
set -e

dir=\$(dirname \$0)

cd \$dir

apt-get -y update 
apt-get -y dist-upgrade
apt-get -y install ssh

echo 'X11UseLocalhost no' >> /etc/ssh/sshd_config

service ssh start

useradd $USER
chsh -s /bin/bash $USER
mkdir -p /home/$USER/.ssh
chown -R $USER:users /home/$USER

cat > $HOME/.ssh/authorized_keys << EOC
$SSHKEY
EOC

chmod 600 $HOME/.ssh/authorized_keys
chown $USER:users $HOME/.ssh/authorized_keys

apt-get -y install ./$UBUNTU/glgrib_${VERSION}_amd64.deb
apt-get -y install ./$UBUNTU/glgrib-data_${VERSION}_amd64.deb

ldd /usr/bin/glgrib

EOF

chmod +x glgrib.sh

sudo docker run -t -d --name ubuntu_glgrib \
  --mount type=bind,src=$PWD/,dst=/root/glgrib,readonly=true \
  ubuntu:$UBUNTU

sudo docker exec ubuntu_glgrib /root/glgrib/glgrib.sh

IP=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" ubuntu_glgrib)
ssh-keygen -f "$HOME/.ssh/known_hosts" -R $IP
ssh -o StrictHostKeyChecking=no -X $IP /usr/bin/glgrib --landscape.on

fi
