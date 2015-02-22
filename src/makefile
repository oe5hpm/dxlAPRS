x86:
	make -f afskmodem.mkf
	make -f udpbox.mkf
	make -f udpgate4.mkf
	make -f udpflex.mkf
	make -f udphub.mkf
	make -f gps2aprs.mkf
	make -f aprsmap.mkf
	make -f sondemod.mkf
	make -f gps2digipos.mkf
	make -f sondeudp.mkf
	make -f udprfnet.mkf

armv6:
	PLATTFORM=armv6 make -f afskmodem.mkf
	PLATTFORM=armv6 make -f udpbox.mkf
	PLATTFORM=armv6 make -f udpgate4.mkf
	PLATTFORM=armv6 make -f udpflex.mkf
	PLATTFORM=armv6 make -f udphub.mkf
	PLATTFORM=armv6 make -f gps2aprs.mkf
	PLATTFORM=armv6 make -f aprsmap.mkf
	PLATTFORM=armv6tce make -f aprsmap.mkf cleanobj
	PLATTFORM=armv6tce make -f aprsmap.mkf
	PLATTFORM=armv6 make -f sondemod.mkf
	PLATTFORM=armv6 make -f gps2digipos.mkf
	PLATTFORM=armv6 make -f sondeudp.mkf
	PLATTFORM=armv6 make -f udprfnet.mkf

armv7hf:
	PLATTFORM=armv7hf make -f afskmodem.mkf
	PLATTFORM=armv7hf make -f udpbox.mkf
	PLATTFORM=armv7hf make -f udpgate4.mkf
	PLATTFORM=armv7hf make -f udpflex.mkf
	PLATTFORM=armv7hf make -f udphub.mkf
	PLATTFORM=armv7hf make -f gps2aprs.mkf
	PLATTFORM=armv7hf make -f aprsmap.mkf
	PLATTFORM=armv7hf make -f sondemod.mkf
	PLATTFORM=armv7hf make -f gps2digipos.mkf
	PLATTFORM=armv7hf make -f sondeudp.mkf
	PLATTFORM=armv7hf make -f udprfnet.mkf

clean:
	make -f afskmodem.mkf clean
	make -f udpbox.mkf clean
	make -f udpgate4.mkf clean
	make -f udpflex.mkf clean
	make -f udphub.mkf clean
	make -f gps2aprs.mkf clean
	make -f aprsmap.mkf clean
	PLATTFORM=armv6tce make -f aprsmap.mkf clean
	make -f sondemod.mkf clean
	make -f gps2digipos.mkf clean
	make -f sondeudp.mkf clean
	make -f udprfnet.mkf clean


