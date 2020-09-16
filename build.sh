if [ 0 = 1 ];then
unzip downloads/gsoap_2.8.106.zip

sudo apt install bison flex openssl

pushd gsoap-2.8
./configure --prefix=$(pwd)/install
make -j
make install
sed -i "s/SOAP_ENV__Fault$/SOAP_ENV__Fault_alex/g" install/share/gsoap/import/wsa5.h
popd

fi


GSOAP_DIR=$(pwd)/gsoap-2.8/install/
mkdir ${GSOAP_DIR}/onvif/  -p
pushd ${GSOAP_DIR}/onvif/
${GSOAP_DIR}/bin/wsdl2h -O4 -P -x -o /tmp/onvif.h https://www.onvif.org/ver10/network/wsdl/remotediscovery.wsdl https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl https://www.onvif.org/ver10/media/wsdl/media.wsdl https://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl 
${GSOAP_DIR}/bin/soapcpp2 -2 -C -L -x -I ${GSOAP_DIR}/share/gsoap/import /tmp/onvif.h
rm RemoteDiscoveryBinding.nsmap PTZBinding.nsmap DeviceBinding.nsmap MediaBinding.nsmap
popd


echo bye
