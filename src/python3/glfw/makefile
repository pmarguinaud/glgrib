
TOP=../../..

all: build

install:
	pip3 install --upgrade -t $(TOP)/debian/tmp/usr/lib/python3/dist-packages/ -r requirements.txt  .

build:
	python3 setup.py build

clean:
	\rm -rf build *.egg-info
