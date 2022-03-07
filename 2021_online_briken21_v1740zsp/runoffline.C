#include "stdio.h"
#include "string.h"
void runoffline(char* input,char* output) {
  gROOT->ProcessLine(".L libmyanalysis.so");
  //gROOT->ProcessLine("gSystem->Load(\"libpmonitor\")");
  gROOT->ProcessLine("pfileopen(\"/home/daq/testdaq/rawdata/preparation/pre0run00001.evt\")");
  //gROOT->ProcessLine("plistopen(\"listfiles.txt\")");
  gROOT->ProcessLine(Form("phsave(\"%s\");",output));
  gROOT->ProcessLine("prun()");
}
