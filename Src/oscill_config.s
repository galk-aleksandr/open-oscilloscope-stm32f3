	AREA    MyBinFile1_Section, DATA, READONLY
	EXPORT OscillConfigDataNaked
	EXPORT OscillConfigDataShielded
OscillConfigDataNaked
    DCD OscillConfigDataNakedExt_End - OscillConfigDataNakedExt
OscillConfigDataNakedExt
	INCBIN nakedConfig.txt
OscillConfigDataNakedExt_End		

OscillConfigDataShielded
    DCD OscillConfigDataShieldedExt_End - OscillConfigDataShieldedExt
OscillConfigDataShieldedExt
	INCBIN shieldedConfig.txt
OscillConfigDataShieldedExt_End		
	END