#include "stdio.h"
#include "string.h"
void daqmonitor() {
  gROOT->ProcessLine(".L libmyanalysis.so");
  gROOT->ProcessLine("gSystem->Load(\"libpmonitor\")");
  //gROOT->ProcessLine("pfileopen(\"/home/daq/testdaq/rawdata/preparation/pre2run00001.evt\")");
  gROOT->ProcessLine("rcdaqopen()");
  gROOT->ProcessLine("pstart()");
}
