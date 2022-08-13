#!/bin/bash


set -x
set -e

if [ 1 -eq 1 ]
then
sudo docker pull ubuntu:latest
sudo docker run -t -d --name ubuntu ubuntu:latest

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

useradd phi001
chsh -s /bin/bash phi001
mkdir -p /home/phi001/.ssh
chown -R phi001:users /home/phi001

cat > /home/phi001/.ssh/authorized_keys << EOC
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCrCCQMzAptwiJfc9SZfr4W1cawOjW8uCzMFNsYXcsc+ESXOlBF4AZhr/4BCQhV5Evc32kJA7LPWKKquqXl6JPcM9+/23DPW3L+N07pxfFskAUVrnaLM7++JZAY4d+PjlowNeBuE9FQIqty5JxPHwb3HJgTIJqJY37+TU3F8cQphWBasfZdN6x+lC9Rfkd/KHLMI/KXhmDu/2mFpeAngKcN6/1Agzf3mgm1WY5Kt0bU5UeiQqeBg6fP07ZplbRsCvqXm1n1WhRw+9LAKsuL95BIZrGcm5B3XdJvuCSesyfz6MiLSEL3Oar7ayghlnJLmoQtOmxSgvAf5I0lqy+RN46mIGoV0tnebvhlM28tx9j+GL1DdSf7i5x9mzWKLRdd8SQkUfT/qxTe7Be1+9fg+236hh4xeh4EfET+39t2Vp8YOehmnt7h28gA2XPDARcmCl4n+E/KcrNvtnga0CYME0sbguah7DXjMxzMKUeUXEPW/Gnc9GLjlQYJGMhsgpeU8pU= phi001@caladan
EOC

chmod 600 /home/phi001/.ssh/authorized_keys
chown phi001:users /home/phi001/.ssh/authorized_keys

apt -y install \$dir/glgrib_1.0-1_amd64.deb
apt -y install \$dir/glgrib-data_1.0-1_amd64.deb

ldd /usr/bin/glgrib

EOF

chmod +x glgrib.sh

for f in glgrib.sh glgrib_1.0-1_amd64.deb glgrib-data_1.0-1_amd64.deb
do 
  sudo docker cp $f ubuntu:/root/$f
done

\rm glgrib.sh

sudo docker exec ubuntu /root/glgrib.sh

IP=$(sudo docker inspect -f "{{ .NetworkSettings.IPAddress }}" ubuntu)
ssh-keygen -f "/home/phi001/.ssh/known_hosts" -R $IP

sudo docker commit ubuntu ubuntu:glgrib

ssh -o StrictHostKeyChecking=no -X 172.17.0.2 /usr/bin/glgrib --landscape.on

fi
