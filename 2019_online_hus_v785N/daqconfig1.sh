#add to begin of run event
rcdaq_client daq_define_runtype beam  /home/daq/testdaq/data/beam/beamrun%05d.evt
rcdaq_client daq_define_runtype preparation  /home/daq/testdaq/data/preparation/preparationrun%05d.evt

#buffer
rcdaq_client  daq_set_adaptivebuffering 0
rcdaq_client  daq_set_maxbuffersize 128

#load caen plugin
rcdaq_client load librcdaqplugin_CAENdrs.so

#loading lupo
rcdaq_client create_device device_caenadc 1 10 0 0 0xED00

#set run type
rcdaq_client daq_set_runtype preparation

