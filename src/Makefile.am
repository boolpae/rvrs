AUTOMAKE_OPTIONS =
bin_PROGRAMS = stas
stas_SOURCES = CallExecutor.cpp HAManager.cpp STT2File.cpp VDClient.cpp VRCManager.cpp stas.cpp CallReceiver.cpp Notifier.cpp Schd4DB.cpp VFCManager.cpp VRClient.cpp CallSignal.cpp STT2DB.cpp VDCManager.cpp VFClient.cpp WorkTracer.cpp

noinst_HEADERS = CallExecutor.h Notifier.h VDCManager.h VRCManager.h stas.h CallReceiver.h STT2DB.h VDClient.h VRClient.h CallSignal.h STT2File.h VFCManager.h WorkTracer.h HAManager.h Schd4DB.h VFClient.h configuration.h

LDADD = ../lib/libitfconfig.a
stas_LDADD = -lzdb -lfvad

$(bin_PROGRAMS): ../lib/libitfconfig.a
