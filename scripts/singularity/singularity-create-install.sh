#!/bin/bash

image=singularity.glgrib-install

cat -> $image.def << EOF
Bootstrap: docker

From: debian:unstable

%files
  /usr/local/share/ca-certificates/meteo-fr.crt /usr/local/share/ca-certificates/meteo-fr.crt
  /usr/local/share/ca-certificates/proxy1.crt   /usr/local/share/ca-certificates/proxy1.crt


%post
  DEBIAN_FRONTEND=noninteractive TZ=Europe/Paris \
  echo 'exit 101' > /usr/sbin/policy-rc.d && \
  chmod +x /usr/sbin/policy-rc.d && \
  dpkg-divert --local --rename --add /usr/bin/systemd-tmpfiles && \
  ln -s /bin/true /usr/bin/systemd-tmpfiles && \
  ln -fs /usr/share/zoneinfo/Europe/Paris /etc/localtime && \
  echo "Europe/Paris" > /etc/timezone && \
  apt update && \
  apt full-upgrade -y && \
  apt install -y command-not-found locales \
    dpkg-dev apt-file git vim ssh screen && \
  update-ca-certificates && \
  sed -i 's/^# *fr_FR.UTF-8 UTF-8/fr_FR.UTF-8 UTF-8/' /etc/locale.gen && \
  locale-gen && \
  apt update && \
  apt-file update && \
  echo 'source /usr/share/modules/init/bash' >> /etc/bash.bashrc && \
  rm -rf /home/ubuntu && \
  echo "deb [trusted=yes] file:/root/glgrib ./" > /etc/apt/sources.list.d/glgrib.list

%environment
  export GIT_SSL_NO_VERIFY=true
  export LANG=fr_FR.UTF-8
  export LANGUAGE=fr_FR:fr
  export LC_ALL=fr_FR.UTF-8

EOF

export TMPDIR=$HOME/tmp
export SINGULARITY_TMPDIR=$HOME/tmp

# singularity build  --fakeroot $image.sif $image.def
singularity build  --fakeroot --sandbox ../$image $image.def


exit 0
singularity shell --fakeroot --writable $image           # root
singularity shell $image                                 # marguina
singularity build --fakeroot $image.sif $image           # creer un .sif a partir du repertoire

