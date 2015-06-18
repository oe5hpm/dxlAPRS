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
	PLATFORM=armv6 make -f afskmodem.mkf
	PLATFORM=armv6 make -f udpbox.mkf
	PLATFORM=armv6 make -f udpgate4.mkf
	PLATFORM=armv6 make -f udpflex.mkf
	PLATFORM=armv6 make -f udphub.mkf
	PLATFORM=armv6 make -f gps2aprs.mkf
	PLATFORM=armv6 make -f aprsmap.mkf
	PLATFORM=armv6tce make -f aprsmap.mkf cleanobj
	PLATFORM=armv6tce make -f aprsmap.mkf
	PLATFORM=armv6 make -f sondemod.mkf
	PLATFORM=armv6 make -f gps2digipos.mkf
	PLATFORM=armv6 make -f sondeudp.mkf
	PLATFORM=armv6 make -f udprfnet.mkf

armv7hf:
	PLATFORM=armv7hf make -f afskmodem.mkf
	PLATFORM=armv7hf make -f udpbox.mkf
	PLATFORM=armv7hf make -f udpgate4.mkf
	PLATFORM=armv7hf make -f udpflex.mkf
	PLATFORM=armv7hf make -f udphub.mkf
	PLATFORM=armv7hf make -f gps2aprs.mkf
	PLATFORM=armv7hf make -f aprsmap.mkf
	PLATFORM=armv7hf make -f sondemod.mkf
	PLATFORM=armv7hf make -f gps2digipos.mkf
	PLATFORM=armv7hf make -f sondeudp.mkf
	PLATFORM=armv7hf make -f udprfnet.mkf

clean:
	make -f afskmodem.mkf clean
	make -f udpbox.mkf clean
	make -f udpgate4.mkf clean
	make -f udpflex.mkf clean
	make -f udphub.mkf clean
	make -f gps2aprs.mkf clean
	make -f aprsmap.mkf clean
	PLATFORM=armv6tce make -f aprsmap.mkf clean
	make -f sondemod.mkf clean
	make -f gps2digipos.mkf clean
	make -f sondeudp.mkf clean
	make -f udprfnet.mkf clean


