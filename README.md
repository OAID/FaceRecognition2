# EAIDK-Demo
[![GitHub license](http://dmlc.github.io/img/apache2.svg)](./LICENSE)

EAIDK-Demo is an implementation project of face detection and  recognition. The face detection using MTCNN algorithm, and recognition using LightenedCNN algorithm. 

The release version is 0.1.0, is based on ROCK960 Platform, target OS is Ubuntu 16.04.

* MTCNN is a deep cascaded multi-task framework to boost up face detection performance. See also [OAID/FaceDetection](https://github.com/OAID/FaceDetection).
* Lightened CNN is a light CNN framework to learn a compact embedding on the large-scale face data with massive noisy labels. See also [LightenedCNN](https://github.com/AlfredXiangWu/face_verification_experiment).

# Release History

### Version 0.1.0 - 2018-6-1
   
  Initial version supports face register, face detection, and face recognization. 
  Support Caffe-HRT and Tengine 

## Build
#### install dependency library 

	sudo apt-get install git cmake scons protobuf-compiler libgflags-dev libgoogle-glog-dev libblas-dev libhdf5-serial-dev liblmdb-dev libleveldb-dev liblapack-dev libsnappy-dev python-numpy libprotobuf-dev libopenblas-dev libgtk2.0-dev python-yaml python-numpy python-scipy python-six
    sudo apt-get install --no-install-recommends libboost-all-dev
    sudo apt-get install libreadline-dev xinetd telnet libtool autoconf wget perl subversion build-essential gfortran libatlas-dev  libatlas-base-dev git build-essential vim-gtk libgtk-3-0 libgtk-3-dev libegl1-mesa-dev
    
    * [Caffe-HRT](https://github.com/OAID/Caffe-HRT) install
	Please see https://github.com/OAID/Caffe-HRT/blob/master/docs/installation.md
	* [Tengine](https://github.com/OAID/Tengine) install
	Please see https://github.com/OAID/Tengine/blob/master/doc/install.md

#### Build the runtime shared libraries

	cd EAIDK-Demo
	make -j4

#### run the demo

	chmod +x ./run.sh
        ./run.sh Tengine
        ./run.sh Caffe-HRT
