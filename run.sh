rm -rf faces/*.png
rm -rf models/face_demo.dat

export OPENBLAS_NUM_THREADS=1
export BYPASSACL=0x0
export CONV_INT_PRIO=2000
export CAFFELIB=/usr/local/AID/Caffe-HRT/lib/
export LD_LIBRARY_PATH=/usr/local/AID/AID-lib

if [ $# == 0 ] ; then
./bin/fp Tengine
else
case $1 in 
	Caffe-HRT) 
		./bin/fp Caffe-HRT
	;; 
	Tengine) 
		./bin/fp Tengine
	;; 
	*) 
		echo "Ignorant" 
	;; 
esac 
fi 
