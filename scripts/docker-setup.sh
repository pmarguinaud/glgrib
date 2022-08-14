#!/bin/bash


set -x
set -e

if [ 1 -eq 1 ]
then
sudo docker pull ubuntu:latest

SSHKEY=$(cat $HOME/.ssh/id_rsa.pub)

cat > glgrib.sh << EOF
#!/bin/bash

set -x
set -e

dir=/root

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

apt-get -y install \$dir/glgrib_1.0-1_amd64.deb
apt-get -y install \$dir/glgrib-data_1.0-1_amd64.deb

ldd /usr/bin/glgrib

EOF

chmod +x glgrib.sh

sudo docker run -t -d --name ubuntu \
  --mount type=bind,src=$PWD/glgrib.sh,dst=/root/glgrib.sh,readonly=true \
  --mount type=bind,src=$PWD/glgrib_1.0-1_amd64.deb,dst=/root/glgrib_1.0-1_amd64.deb,readonly=true \
  --mount type=bind,src=$PWD/glgrib-data_1.0-1_amd64.deb,dst=/root/glgrib-data_1.0-1_amd64.deb,readonly=true \
  ubuntu:latest

\rm glgrib.sh

sudo docker exec ubuntu /root/glgrib.sh

IP=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" ubuntu)
ssh-keygen -f "$HOME/.ssh/known_hosts" -R $IP

#sudo docker commit ubuntu ubuntu:glgrib

ssh -o StrictHostKeyChecking=no -X 172.17.0.2 /usr/bin/glgrib --landscape.on

fi
